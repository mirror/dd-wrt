/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2001-2016 Neil Brown <neilb@suse.com>
 *
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Author: Neil Brown
 *    Email: <neilb@suse.de>
 */

#include <stddef.h>
#include "mdadm.h"
/*
 * The version-1 superblock :
 * All numeric fields are little-endian.
 *
 * total size: 256 bytes plus 2 per device.
 *  1K allows 384 devices.
 */
struct mdp_superblock_1 {
	/* constant array information - 128 bytes */
	__u32	magic;		/* MD_SB_MAGIC: 0xa92b4efc - little endian */
	__u32	major_version;	/* 1 */
	__u32	feature_map;	/* 0 for now */
	__u32	pad0;		/* always set to 0 when writing */

	__u8	set_uuid[16];	/* user-space generated. */
	char	set_name[32];	/* set and interpreted by user-space */

	__u64	ctime;		/* lo 40 bits are seconds, top 24 are microseconds or 0*/
	__u32	level;		/* -4 (multipath), -1 (linear), 0,1,4,5 */
	__u32	layout;		/* only for raid5 currently */
	__u64	size;		/* used size of component devices, in 512byte sectors */

	__u32	chunksize;	/* in 512byte sectors */
	__u32	raid_disks;
	union {
		__u32	bitmap_offset;	/* sectors after start of superblock that bitmap starts
					 * NOTE: signed, so bitmap can be before superblock
					 * only meaningful of feature_map[0] is set.
					 */

		/* only meaningful when feature_map[MD_FEATURE_PPL] is set */
		struct {
			__s16 offset; /* sectors from start of superblock that ppl starts */
			__u16 size; /* ppl size in sectors */
		} ppl;
	};

	/* These are only valid with feature bit '4' */
	__u32	new_level;	/* new level we are reshaping to		*/
	__u64	reshape_position;	/* next address in array-space for reshape */
	__u32	delta_disks;	/* change in number of raid_disks		*/
	__u32	new_layout;	/* new layout					*/
	__u32	new_chunk;	/* new chunk size (sectors)			*/
	__u32	new_offset;	/* signed number to add to data_offset in new
				 * layout.  0 == no-change.  This can be
				 * different on each device in the array.
				 */

	/* constant this-device information - 64 bytes */
	__u64	data_offset;	/* sector start of data, often 0 */
	__u64	data_size;	/* sectors in this device that can be used for data */
	__u64	super_offset;	/* sector start of this superblock */
	union {
		__u64	recovery_offset;/* sectors before this offset (from data_offset) have been recovered */
		__u64	journal_tail;/* journal tail of journal device (from data_offset) */
	};
	__u32	dev_number;	/* permanent identifier of this  device - not role in raid */
	__u32	cnt_corrected_read; /* number of read errors that were corrected by re-writing */
	__u8	device_uuid[16]; /* user-space setable, ignored by kernel */
	__u8    devflags;        /* per-device flags.  Only one defined...*/
#define WriteMostly1    1        /* mask for writemostly flag in above */
#define FailFast1	2        /* Device should get FailFast requests */
	/* bad block log.  If there are any bad blocks the feature flag is set.
	 * if offset and size are non-zero, that space is reserved and available.
	 */
	__u8	bblog_shift;	/* shift from sectors to block size for badblock list */
	__u16	bblog_size;	/* number of sectors reserved for badblock list */
	__u32	bblog_offset;	/* sector offset from superblock to bblog, signed */

	/* array state information - 64 bytes */
	__u64	utime;		/* 40 bits second, 24 bits microseconds */
	__u64	events;		/* incremented when superblock updated */
	__u64	resync_offset;	/* data before this offset (from data_offset) known to be in sync */
	__u32	sb_csum;	/* checksum upto dev_roles[max_dev] */
	__u32	max_dev;	/* size of dev_roles[] array to consider */
	__u8	pad3[64-32];	/* set to 0 when writing */

	/* device state information. Indexed by dev_number.
	 * 2 bytes per device
	 * Note there are no per-device state flags. State information is rolled
	 * into the 'roles' value.  If a device is spare or faulty, then it doesn't
	 * have a meaningful role.
	 */
	__u16	dev_roles[0];	/* role in array, or 0xffff for a spare, or 0xfffe for faulty */
};

#define MAX_SB_SIZE 4096
/* bitmap super size is 256, but we round up to a sector for alignment */
#define BM_SUPER_SIZE 512
#define MAX_DEVS ((int)(MAX_SB_SIZE - sizeof(struct mdp_superblock_1)) / 2)
#define SUPER1_SIZE	(MAX_SB_SIZE + BM_SUPER_SIZE \
			 + sizeof(struct misc_dev_info))

struct misc_dev_info {
	__u64 device_size;
};

#define MULTIPLE_PPL_AREA_SIZE_SUPER1 (1024 * 1024) /* Size of the whole
						     * mutliple PPL area
						     */
/* feature_map bits */
#define MD_FEATURE_BITMAP_OFFSET	1
#define	MD_FEATURE_RECOVERY_OFFSET	2 /* recovery_offset is present and
					   * must be honoured
					   */
#define	MD_FEATURE_RESHAPE_ACTIVE	4
#define	MD_FEATURE_BAD_BLOCKS		8 /* badblock list is not empty */
#define	MD_FEATURE_REPLACEMENT		16 /* This device is replacing an
					    * active device with same 'role'.
					    * 'recovery_offset' is also set.
					    */
#define	MD_FEATURE_RESHAPE_BACKWARDS	32 /* Reshape doesn't change number
					    * of devices, but is going
					    * backwards anyway.
					    */
#define	MD_FEATURE_NEW_OFFSET		64 /* new_offset must be honoured */
#define	MD_FEATURE_BITMAP_VERSIONED	256 /* bitmap version number checked properly */
#define	MD_FEATURE_JOURNAL		512 /* support write journal */
#define	MD_FEATURE_PPL			1024 /* support PPL */
#define	MD_FEATURE_MUTLIPLE_PPLS	2048 /* support for multiple PPLs */
#define	MD_FEATURE_ALL			(MD_FEATURE_BITMAP_OFFSET	\
					|MD_FEATURE_RECOVERY_OFFSET	\
					|MD_FEATURE_RESHAPE_ACTIVE	\
					|MD_FEATURE_BAD_BLOCKS		\
					|MD_FEATURE_REPLACEMENT		\
					|MD_FEATURE_RESHAPE_BACKWARDS	\
					|MD_FEATURE_NEW_OFFSET		\
					|MD_FEATURE_BITMAP_VERSIONED	\
					|MD_FEATURE_JOURNAL		\
					|MD_FEATURE_PPL			\
					|MD_FEATURE_MULTIPLE_PPLS	\
					)

static int role_from_sb(struct mdp_superblock_1 *sb)
{
	unsigned int d;
	int role;

	d = __le32_to_cpu(sb->dev_number);
	if (d < __le32_to_cpu(sb->max_dev))
		role = __le16_to_cpu(sb->dev_roles[d]);
	else
		role = MD_DISK_ROLE_SPARE;
	return role;
}

/* return how many bytes are needed for bitmap, for cluster-md each node
 * should have it's own bitmap */
static unsigned int calc_bitmap_size(bitmap_super_t *bms, unsigned int boundary)
{
	unsigned long long bits, bytes;

	bits = bitmap_bits(__le64_to_cpu(bms->sync_size),
			   __le32_to_cpu(bms->chunksize));
	bytes = (bits+7) >> 3;
	bytes += sizeof(bitmap_super_t);
	bytes = ROUND_UP(bytes, boundary);

	return bytes;
}

static unsigned int calc_sb_1_csum(struct mdp_superblock_1 * sb)
{
	unsigned int disk_csum, csum;
	unsigned long long newcsum;
	int size = sizeof(*sb) + __le32_to_cpu(sb->max_dev)*2;
	unsigned int *isuper = (unsigned int*)sb;

/* make sure I can count... */
	if (offsetof(struct mdp_superblock_1,data_offset) != 128 ||
	    offsetof(struct mdp_superblock_1, utime) != 192 ||
	    sizeof(struct mdp_superblock_1) != 256) {
		fprintf(stderr, "WARNING - superblock isn't sized correctly\n");
	}

	disk_csum = sb->sb_csum;
	sb->sb_csum = 0;
	newcsum = 0;
	for (; size>=4; size -= 4 ) {
		newcsum += __le32_to_cpu(*isuper);
		isuper++;
	}

	if (size == 2)
		newcsum += __le16_to_cpu(*(unsigned short*) isuper);

	csum = (newcsum & 0xffffffff) + (newcsum >> 32);
	sb->sb_csum = disk_csum;
	return __cpu_to_le32(csum);
}

/*
 * Information related to file descriptor used for aligned reads/writes.
 * Cache the block size.
 */
struct align_fd {
	int fd;
	int blk_sz;
};

static void init_afd(struct align_fd *afd, int fd)
{
	afd->fd = fd;
	if (!get_dev_sector_size(afd->fd, NULL, (unsigned int *)&afd->blk_sz))
		afd->blk_sz = 512;
}

static char abuf[4096+4096];

static int aread(struct align_fd *afd, void *buf, int len)
{
	/* aligned read.
	 * On devices with a 4K sector size, we need to read
	 * the full sector and copy relevant bits into
	 * the buffer
	 */
	int bsize, iosize;
	char *b;
	int n;

	bsize = afd->blk_sz;

	if (!bsize || bsize > 4096 || len > 4096) {
		if (!bsize)
			fprintf(stderr, "WARNING - aread() called with invalid block size\n");
		return -1;
	}
	b = ROUND_UP_PTR((char *)abuf, 4096);

	for (iosize = 0; iosize < len; iosize += bsize)
		;
	n = read(afd->fd, b, iosize);
	if (n <= 0)
		return n;
	lseek(afd->fd, len - n, 1);
	if (n > len)
		n = len;
	memcpy(buf, b, n);
	return n;
}

static int awrite(struct align_fd *afd, void *buf, int len)
{
	/* aligned write.
	 * On devices with a 4K sector size, we need to write
	 * the full sector.  We pre-read if the sector is larger
	 * than the write.
	 * The address must be sector-aligned.
	 */
	int bsize, iosize;
	char *b;
	int n;

	bsize = afd->blk_sz;
	if (!bsize || bsize > 4096 || len > 4096) {
		if (!bsize)
			fprintf(stderr, "WARNING - awrite() called with invalid block size\n");
		return -1;
	}
	b = ROUND_UP_PTR((char *)abuf, 4096);

	for (iosize = 0; iosize < len ; iosize += bsize)
		;

	if (len != iosize) {
		n = read(afd->fd, b, iosize);
		if (n <= 0)
			return n;
		lseek(afd->fd, -n, 1);
	}

	memcpy(b, buf, len);
	n = write(afd->fd, b, iosize);
	if (n <= 0)
		return n;
	lseek(afd->fd, len - n, 1);
	return len;
}

static inline unsigned int md_feature_any_ppl_on(__u32 feature_map)
{
	return ((__cpu_to_le32(feature_map) &
	    (MD_FEATURE_PPL | MD_FEATURE_MUTLIPLE_PPLS)));
}

static inline unsigned int choose_ppl_space(int chunk)
{
	return (PPL_HEADER_SIZE >> 9) + (chunk > 128*2 ? chunk : 128*2);
}

static void examine_super1(struct supertype *st, char *homehost)
{
	struct mdp_superblock_1 *sb = st->sb;
	bitmap_super_t *bms = (bitmap_super_t*)(((char*)sb)+MAX_SB_SIZE);
	time_t atime;
	unsigned int d;
	int role;
	int delta_extra = 0;
	int i;
	char *c;
	int l = homehost ? strlen(homehost) : 0;
	int layout;
	unsigned long long sb_offset;
	struct mdinfo info;

	printf("          Magic : %08x\n", __le32_to_cpu(sb->magic));
	printf("        Version : 1");
	sb_offset = __le64_to_cpu(sb->super_offset);
	if (sb_offset <= 4)
		printf(".1\n");
	else if (sb_offset <= 8)
		printf(".2\n");
	else
		printf(".0\n");
	printf("    Feature Map : 0x%x\n", __le32_to_cpu(sb->feature_map));
	printf("     Array UUID : ");
	for (i=0; i<16; i++) {
		if ((i&3)==0 && i != 0) printf(":");
		printf("%02x", sb->set_uuid[i]);
	}
	printf("\n");
	printf("           Name : %.32s", sb->set_name);
	if (l > 0 && l < 32 &&
	    sb->set_name[l] == ':' &&
	    strncmp(sb->set_name, homehost, l) == 0)
		printf("  (local to host %s)", homehost);
	printf("\n");
	if (bms->nodes > 0 &&
	    (__le32_to_cpu(sb->feature_map) & MD_FEATURE_BITMAP_OFFSET))
		printf("   Cluster Name : %-64s\n", bms->cluster_name);
	atime = __le64_to_cpu(sb->ctime) & 0xFFFFFFFFFFULL;
	printf("  Creation Time : %.24s\n", ctime(&atime));
	c=map_num(pers, __le32_to_cpu(sb->level));
	printf("     Raid Level : %s\n", c?c:"-unknown-");
	printf("   Raid Devices : %d\n", __le32_to_cpu(sb->raid_disks));
	printf("\n");
	printf(" Avail Dev Size : %llu%s\n",
	       (unsigned long long)__le64_to_cpu(sb->data_size),
	       human_size(__le64_to_cpu(sb->data_size)<<9));
	if (__le32_to_cpu(sb->level) > 0) {
		int ddsks = 0, ddsks_denom = 1;
		switch(__le32_to_cpu(sb->level)) {
		case 1: ddsks=1;break;
		case 4:
		case 5: ddsks = __le32_to_cpu(sb->raid_disks)-1; break;
		case 6: ddsks = __le32_to_cpu(sb->raid_disks)-2; break;
		case 10:
			layout = __le32_to_cpu(sb->layout);
			ddsks = __le32_to_cpu(sb->raid_disks);
			ddsks_denom = (layout&255) * ((layout>>8)&255);
		}
		if (ddsks) {
			long long asize = __le64_to_cpu(sb->size);
			asize = (asize << 9) * ddsks / ddsks_denom;
			printf("     Array Size : %llu%s\n",
			       asize >> 10,  human_size(asize));
		}
		if (sb->size != sb->data_size)
			printf("  Used Dev Size : %llu%s\n",
			       (unsigned long long)__le64_to_cpu(sb->size),
			       human_size(__le64_to_cpu(sb->size)<<9));
	}
	if (sb->data_offset)
		printf("    Data Offset : %llu sectors\n",
		       (unsigned long long)__le64_to_cpu(sb->data_offset));
	if (sb->new_offset &&
	    (__le32_to_cpu(sb->feature_map) & MD_FEATURE_NEW_OFFSET)) {
		unsigned long long offset = __le64_to_cpu(sb->data_offset);
		offset += (signed)(int32_t)__le32_to_cpu(sb->new_offset);
		printf("     New Offset : %llu sectors\n", offset);
	}
	printf("   Super Offset : %llu sectors\n",
	       (unsigned long long)__le64_to_cpu(sb->super_offset));
	if (__le32_to_cpu(sb->feature_map) & MD_FEATURE_RECOVERY_OFFSET)
		printf("Recovery Offset : %llu sectors\n",
		       (unsigned long long)__le64_to_cpu(sb->recovery_offset));

	st->ss->getinfo_super(st, &info, NULL);
	if (info.space_after != 1 &&
	    !(__le32_to_cpu(sb->feature_map) & MD_FEATURE_NEW_OFFSET))
		printf("   Unused Space : before=%llu sectors, after=%llu sectors\n",
		       info.space_before, info.space_after);

	printf("          State : %s\n",
	       (__le64_to_cpu(sb->resync_offset)+1)? "active":"clean");
	printf("    Device UUID : ");
	for (i=0; i<16; i++) {
		if ((i&3)==0 && i != 0)
			printf(":");
		printf("%02x", sb->device_uuid[i]);
	}
	printf("\n");
	printf("\n");
	if (sb->feature_map & __cpu_to_le32(MD_FEATURE_BITMAP_OFFSET)) {
		printf("Internal Bitmap : %ld sectors from superblock\n",
		       (long)(int32_t)__le32_to_cpu(sb->bitmap_offset));
	} else if (md_feature_any_ppl_on(sb->feature_map)) {
		printf("            PPL : %u sectors at offset %d sectors from superblock\n",
		       __le16_to_cpu(sb->ppl.size),
		       __le16_to_cpu(sb->ppl.offset));
	}
	if (sb->feature_map & __cpu_to_le32(MD_FEATURE_RESHAPE_ACTIVE)) {
		printf("  Reshape pos'n : %llu%s\n", (unsigned long long)
		       __le64_to_cpu(sb->reshape_position)/2,
		       human_size(__le64_to_cpu(sb->reshape_position)<<9));
		if (__le32_to_cpu(sb->delta_disks)) {
			printf("  Delta Devices : %d",
			       __le32_to_cpu(sb->delta_disks));
			printf(" (%d->%d)\n",
			       __le32_to_cpu(sb->raid_disks) -
			       __le32_to_cpu(sb->delta_disks),
			       __le32_to_cpu(sb->raid_disks));
			if ((int)__le32_to_cpu(sb->delta_disks) < 0)
				delta_extra = -__le32_to_cpu(sb->delta_disks);
		}
		if (__le32_to_cpu(sb->new_level) != __le32_to_cpu(sb->level)) {
			c = map_num(pers, __le32_to_cpu(sb->new_level));
			printf("      New Level : %s\n", c?c:"-unknown-");
		}
		if (__le32_to_cpu(sb->new_layout) !=
		    __le32_to_cpu(sb->layout)) {
			if (__le32_to_cpu(sb->level) == 5) {
				c = map_num(r5layout,
					    __le32_to_cpu(sb->new_layout));
				printf("     New Layout : %s\n", c?c:"-unknown-");
			}
			if (__le32_to_cpu(sb->level) == 6) {
				c = map_num(r6layout,
					    __le32_to_cpu(sb->new_layout));
				printf("     New Layout : %s\n", c?c:"-unknown-");
			}
			if (__le32_to_cpu(sb->level) == 10) {
				printf("     New Layout :");
				print_r10_layout(__le32_to_cpu(sb->new_layout));
				printf("\n");
			}
		}
		if (__le32_to_cpu(sb->new_chunk) !=
		    __le32_to_cpu(sb->chunksize))
			printf("  New Chunksize : %dK\n",
			       __le32_to_cpu(sb->new_chunk)/2);
		printf("\n");
	}
	if (sb->devflags) {
		printf("          Flags :");
		if (sb->devflags & WriteMostly1)
			printf(" write-mostly");
		if (sb->devflags & FailFast1)
			printf(" failfast");
		printf("\n");
	}

	atime = __le64_to_cpu(sb->utime) & 0xFFFFFFFFFFULL;
	printf("    Update Time : %.24s\n", ctime(&atime));

	if (sb->bblog_size && sb->bblog_offset) {
		printf("  Bad Block Log : %d entries available at offset %ld sectors",
		       __le16_to_cpu(sb->bblog_size)*512/8,
		       (long)(int32_t)__le32_to_cpu(sb->bblog_offset));
		if (sb->feature_map & __cpu_to_le32(MD_FEATURE_BAD_BLOCKS))
			printf(" - bad blocks present.");
		printf("\n");
	}

	if (calc_sb_1_csum(sb) == sb->sb_csum)
		printf("       Checksum : %x - correct\n",
		       __le32_to_cpu(sb->sb_csum));
	else
		printf("       Checksum : %x - expected %x\n",
		       __le32_to_cpu(sb->sb_csum),
		       __le32_to_cpu(calc_sb_1_csum(sb)));
	printf("         Events : %llu\n",
	       (unsigned long long)__le64_to_cpu(sb->events));
	printf("\n");
	if (__le32_to_cpu(sb->level) == 5) {
		c = map_num(r5layout, __le32_to_cpu(sb->layout));
		printf("         Layout : %s\n", c?c:"-unknown-");
	}
	if (__le32_to_cpu(sb->level) == 6) {
		c = map_num(r6layout, __le32_to_cpu(sb->layout));
		printf("         Layout : %s\n", c?c:"-unknown-");
	}
	if (__le32_to_cpu(sb->level) == 10) {
		int lo = __le32_to_cpu(sb->layout);
		printf("         Layout :");
		print_r10_layout(lo);
		printf("\n");
	}
	switch(__le32_to_cpu(sb->level)) {
	case 0:
	case 4:
	case 5:
	case 6:
	case 10:
		printf("     Chunk Size : %dK\n",
		       __le32_to_cpu(sb->chunksize)/2);
		break;
	case -1:
		printf("       Rounding : %dK\n",
		       __le32_to_cpu(sb->chunksize)/2);
		break;
	default:
		break;
	}
	printf("\n");
#if 0
	/* This turns out to just be confusing */
	printf("    Array Slot : %d (", __le32_to_cpu(sb->dev_number));
	for (i = __le32_to_cpu(sb->max_dev); i> 0 ; i--)
		if (__le16_to_cpu(sb->dev_roles[i-1]) != MD_DISK_ROLE_SPARE)
			break;
	for (d = 0; d < i; d++) {
		int role = __le16_to_cpu(sb->dev_roles[d]);
		if (d)
			printf(", ");
		if (role == MD_DISK_ROLE_SPARE)
			printf("empty");
		else
			if(role == MD_DISK_ROLE_FAULTY)
				printf("failed");
			else
				printf("%d", role);
	}
	printf(")\n");
#endif
	printf("   Device Role : ");
	role = role_from_sb(sb);
	if (role >= MD_DISK_ROLE_FAULTY)
		printf("spare\n");
	else if (role == MD_DISK_ROLE_JOURNAL)
		printf("Journal\n");
	else if (sb->feature_map & __cpu_to_le32(MD_FEATURE_REPLACEMENT))
		printf("Replacement device %d\n", role);
	else
		printf("Active device %d\n", role);

	printf("   Array State : ");
	for (d = 0; d < __le32_to_cpu(sb->raid_disks) + delta_extra; d++) {
		int cnt = 0;
		unsigned int i;
		for (i = 0; i < __le32_to_cpu(sb->max_dev); i++) {
			unsigned int role = __le16_to_cpu(sb->dev_roles[i]);
			if (role == d)
				cnt++;
		}
		if (cnt == 2)
			printf("R");
		else if (cnt == 1)
			printf("A");
		else if (cnt == 0)
			printf(".");
		else
			printf("?");
	}
#if 0
	/* This is confusing too */
	faulty = 0;
	for (i = 0; i< __le32_to_cpu(sb->max_dev); i++) {
		int role = __le16_to_cpu(sb->dev_roles[i]);
		if (role == MD_DISK_ROLE_FAULTY)
			faulty++;
	}
	if (faulty)
		printf(" %d failed", faulty);
#endif
	printf(" ('A' == active, '.' == missing, 'R' == replacing)");
	printf("\n");
}

static void brief_examine_super1(struct supertype *st, int verbose)
{
	struct mdp_superblock_1 *sb = st->sb;
	int i;
	unsigned long long sb_offset;
	char *nm;
	char *c = map_num(pers, __le32_to_cpu(sb->level));

	nm = strchr(sb->set_name, ':');
	if (nm)
		nm++;
	else if (sb->set_name[0])
		nm = sb->set_name;
	else
		nm = NULL;

	printf("ARRAY ");
	if (nm) {
		printf("/dev/md/");
		print_escape(nm);
		putchar(' ');
	}
	if (verbose && c)
		printf(" level=%s", c);
	sb_offset = __le64_to_cpu(sb->super_offset);
	if (sb_offset <= 4)
		printf(" metadata=1.1 ");
	else if (sb_offset <= 8)
		printf(" metadata=1.2 ");
	else
		printf(" metadata=1.0 ");
	if (verbose)
		printf("num-devices=%d ", __le32_to_cpu(sb->raid_disks));
	printf("UUID=");
	for (i = 0; i < 16; i++) {
		if ((i&3)==0 && i != 0)
			printf(":");
		printf("%02x", sb->set_uuid[i]);
	}
	if (sb->set_name[0]) {
		printf(" name=");
		print_quoted(sb->set_name);
	}
	printf("\n");
}

static void export_examine_super1(struct supertype *st)
{
	struct mdp_superblock_1 *sb = st->sb;
	int i;
	int len = 32;
	int layout;

	printf("MD_LEVEL=%s\n", map_num(pers, __le32_to_cpu(sb->level)));
	printf("MD_DEVICES=%d\n", __le32_to_cpu(sb->raid_disks));
	for (i = 0; i < 32; i++)
		if (sb->set_name[i] == '\n' || sb->set_name[i] == '\0') {
			len = i;
			break;
		}
	if (len)
		printf("MD_NAME=%.*s\n", len, sb->set_name);
	if (__le32_to_cpu(sb->level) > 0) {
		int ddsks = 0, ddsks_denom = 1;
		switch(__le32_to_cpu(sb->level)) {
			case 1:
				ddsks = 1;
				break;
			case 4:
			case 5:
				ddsks = __le32_to_cpu(sb->raid_disks)-1;
				break;
			case 6:
				ddsks = __le32_to_cpu(sb->raid_disks)-2;
				break;
			case 10:
				layout = __le32_to_cpu(sb->layout);
				ddsks = __le32_to_cpu(sb->raid_disks);
				ddsks_denom = (layout&255) * ((layout>>8)&255);
			}
		if (ddsks) {
			long long asize = __le64_to_cpu(sb->size);
			asize = (asize << 9) * ddsks / ddsks_denom;
			printf("MD_ARRAY_SIZE=%s\n",
			       human_size_brief(asize, JEDEC));
		}
	}
	printf("MD_UUID=");
	for (i = 0; i < 16; i++) {
		if ((i&3) == 0 && i != 0)
			printf(":");
		printf("%02x", sb->set_uuid[i]);
	}
	printf("\n");
	printf("MD_UPDATE_TIME=%llu\n",
	       __le64_to_cpu(sb->utime) & 0xFFFFFFFFFFULL);
	printf("MD_DEV_UUID=");
	for (i = 0; i < 16; i++) {
		if ((i&3) == 0 && i != 0)
			printf(":");
		printf("%02x", sb->device_uuid[i]);
	}
	printf("\n");
	printf("MD_EVENTS=%llu\n",
	       (unsigned long long)__le64_to_cpu(sb->events));
}

static int copy_metadata1(struct supertype *st, int from, int to)
{
	/* Read superblock.  If it looks good, write it out.
	 * Then if a bitmap is present, copy that.
	 * And if a bad-block-list is present, copy that too.
	 */
	void *buf;
	unsigned long long dsize, sb_offset;
	const int bufsize = 4*1024;
	struct mdp_superblock_1 super, *sb;

	if (posix_memalign(&buf, 4096, bufsize) != 0)
		return 1;

	if (!get_dev_size(from, NULL, &dsize))
		goto err;

	dsize >>= 9;
	if (dsize < 24)
		goto err;
	switch(st->minor_version) {
	case 0:
		sb_offset = dsize;
		sb_offset -= 8*2;
		sb_offset &= ~(4*2-1);
		break;
	case 1:
		sb_offset = 0;
		break;
	case 2:
		sb_offset = 4*2;
		break;
	default:
		goto err;
	}

	if (lseek64(from, sb_offset << 9, 0) < 0LL)
		goto err;
	if (read(from, buf, bufsize) != bufsize)
		goto err;

	sb = buf;
	super = *sb; // save most of sb for when we reuse buf

	if (__le32_to_cpu(super.magic) != MD_SB_MAGIC ||
	    __le32_to_cpu(super.major_version) != 1 ||
	    __le64_to_cpu(super.super_offset) != sb_offset ||
	    calc_sb_1_csum(sb) != super.sb_csum)
		goto err;

	if (lseek64(to, sb_offset << 9, 0) < 0LL)
		goto err;
	if (write(to, buf, bufsize) != bufsize)
		goto err;

	if (super.feature_map & __le32_to_cpu(MD_FEATURE_BITMAP_OFFSET)) {
		unsigned long long bitmap_offset = sb_offset;
		int bytes = 4096; // just an estimate.
		int written = 0;
		struct align_fd afrom, ato;

		init_afd(&afrom, from);
		init_afd(&ato, to);

		bitmap_offset += (int32_t)__le32_to_cpu(super.bitmap_offset);

		if (lseek64(from, bitmap_offset<<9, 0) < 0)
			goto err;
		if (lseek64(to, bitmap_offset<<9, 0) < 0)
			goto err;

		for (written = 0; written < bytes ; ) {
			int n = bytes - written;
			if (n > 4096)
				n = 4096;
			if (aread(&afrom, buf, n) != n)
				goto err;
			if (written == 0) {
				/* have the header, can calculate
				 * correct bitmap bytes */
				bitmap_super_t *bms;
				bms = (void*)buf;
				bytes = calc_bitmap_size(bms, 512);
				if (n > bytes)
					n =  bytes;
			}
			if (awrite(&ato, buf, n) != n)
				goto err;
			written += n;
		}
	}

	if (super.bblog_size != 0 &&
	    __le16_to_cpu(super.bblog_size) <= 100 &&
	    super.bblog_offset != 0 &&
	    (super.feature_map & __le32_to_cpu(MD_FEATURE_BAD_BLOCKS))) {
		/* There is a bad block log */
		unsigned long long bb_offset = sb_offset;
		int bytes = __le16_to_cpu(super.bblog_size) * 512;
		int written = 0;
		struct align_fd afrom, ato;

		init_afd(&afrom, from);
		init_afd(&ato, to);

		bb_offset += (int32_t)__le32_to_cpu(super.bblog_offset);

		if (lseek64(from, bb_offset<<9, 0) < 0)
			goto err;
		if (lseek64(to, bb_offset<<9, 0) < 0)
			goto err;

		for (written = 0; written < bytes ; ) {
			int n = bytes - written;
			if (n > 4096)
				n = 4096;
			if (aread(&afrom, buf, n) != n)
				goto err;

			if (awrite(&ato, buf, n) != n)
				goto err;
			written += n;
		}
	}

	free(buf);
	return 0;

err:
	free(buf);
	return 1;
}

static void detail_super1(struct supertype *st, char *homehost)
{
	struct mdp_superblock_1 *sb = st->sb;
	bitmap_super_t *bms = (bitmap_super_t*)(((char*)sb) + MAX_SB_SIZE);
	int i;
	int l = homehost ? strlen(homehost) : 0;

	printf("              Name : %.32s", sb->set_name);
	if (l > 0 && l < 32 && sb->set_name[l] == ':' &&
	    strncmp(sb->set_name, homehost, l) == 0)
		printf("  (local to host %s)", homehost);
	if (bms->nodes > 0 &&
	    (__le32_to_cpu(sb->feature_map) & MD_FEATURE_BITMAP_OFFSET))
		printf("\n      Cluster Name : %-64s", bms->cluster_name);
	printf("\n              UUID : ");
	for (i = 0; i < 16; i++) {
		if ((i&3) == 0 && i != 0)
			printf(":");
		printf("%02x", sb->set_uuid[i]);
	}
	printf("\n            Events : %llu\n\n",
	       (unsigned long long)__le64_to_cpu(sb->events));
}

static void brief_detail_super1(struct supertype *st)
{
	struct mdp_superblock_1 *sb = st->sb;
	int i;

	if (sb->set_name[0]) {
		printf(" name=");
		print_quoted(sb->set_name);
	}
	printf(" UUID=");
	for (i = 0; i < 16; i++) {
		if ((i & 3) == 0 && i != 0)
			printf(":");
		printf("%02x", sb->set_uuid[i]);
	}
}

static void export_detail_super1(struct supertype *st)
{
	struct mdp_superblock_1 *sb = st->sb;
	int i;
	int len = 32;

	for (i = 0; i < 32; i++)
		if (sb->set_name[i] == '\n' || sb->set_name[i] == '\0') {
			len = i;
			break;
		}
	if (len)
		printf("MD_NAME=%.*s\n", len, sb->set_name);
}

static int examine_badblocks_super1(struct supertype *st, int fd, char *devname)
{
	struct mdp_superblock_1 *sb = st->sb;
	unsigned long long offset;
	int size;
	__u64 *bbl, *bbp;
	int i;

	if  (!sb->bblog_size || __le16_to_cpu(sb->bblog_size) > 100 ||
	     !sb->bblog_offset){
		printf("No bad-blocks list configured on %s\n", devname);
		return 0;
	}
	if ((sb->feature_map & __cpu_to_le32(MD_FEATURE_BAD_BLOCKS)) == 0) {
		printf("Bad-blocks list is empty in %s\n", devname);
		return 0;
	}

	size = __le16_to_cpu(sb->bblog_size)* 512;
	if (posix_memalign((void**)&bbl, 4096, size) != 0) {
		pr_err("could not allocate badblocks list\n");
		return 0;
	}
	offset = __le64_to_cpu(sb->super_offset) +
		(int)__le32_to_cpu(sb->bblog_offset);
	offset <<= 9;
	if (lseek64(fd, offset, 0) < 0) {
		pr_err("Cannot seek to bad-blocks list\n");
		return 1;
	}
	if (read(fd, bbl, size) != size) {
		pr_err("Cannot read bad-blocks list\n");
		return 1;
	}
	/* 64bits per entry. 10 bits is block-count, 54 bits is block
	 * offset.  Blocks are sectors unless bblog->shift makes them bigger
	 */
	bbp = (__u64*)bbl;
	printf("Bad-blocks on %s:\n", devname);
	for (i = 0; i < size/8; i++, bbp++) {
		__u64 bb = __le64_to_cpu(*bbp);
		int count = bb & 0x3ff;
		unsigned long long sector = bb >> 10;

		if (bb + 1 == 0)
			break;

		sector <<= sb->bblog_shift;
		count <<= sb->bblog_shift;

		printf("%20llu for %d sectors\n", sector, count);
	}
	return 0;
}

static int match_home1(struct supertype *st, char *homehost)
{
	struct mdp_superblock_1 *sb = st->sb;
	int l = homehost ? strlen(homehost) : 0;

	return (l > 0 && l < 32 && sb->set_name[l] == ':' &&
		strncmp(sb->set_name, homehost, l) == 0);
}

static void uuid_from_super1(struct supertype *st, int uuid[4])
{
	struct mdp_superblock_1 *super = st->sb;
	char *cuuid = (char*)uuid;
	int i;
	for (i = 0; i < 16; i++)
		cuuid[i] = super->set_uuid[i];
}

static void getinfo_super1(struct supertype *st, struct mdinfo *info, char *map)
{
	struct mdp_superblock_1 *sb = st->sb;
	struct bitmap_super_s *bsb = (void*)(((char*)sb)+MAX_SB_SIZE);
	struct misc_dev_info *misc =
		(void*)(((char*)sb)+MAX_SB_SIZE+BM_SUPER_SIZE);
	int working = 0;
	unsigned int i;
	unsigned int role;
	unsigned int map_disks = info->array.raid_disks;
	unsigned long long super_offset;
	unsigned long long data_size;

	memset(info, 0, sizeof(*info));
	info->array.major_version = 1;
	info->array.minor_version = st->minor_version;
	info->array.patch_version = 0;
	info->array.raid_disks = __le32_to_cpu(sb->raid_disks);
	info->array.level = __le32_to_cpu(sb->level);
	info->array.layout = __le32_to_cpu(sb->layout);
	info->array.md_minor = -1;
	info->array.ctime = __le64_to_cpu(sb->ctime);
	info->array.utime = __le64_to_cpu(sb->utime);
	info->array.chunk_size = __le32_to_cpu(sb->chunksize)*512;
	info->array.state =
		(__le64_to_cpu(sb->resync_offset) == MaxSector)	? 1 : 0;

	super_offset = __le64_to_cpu(sb->super_offset);
	info->data_offset = __le64_to_cpu(sb->data_offset);
	info->component_size = __le64_to_cpu(sb->size);
	if (sb->feature_map & __le32_to_cpu(MD_FEATURE_BITMAP_OFFSET)) {
		info->bitmap_offset = (int32_t)__le32_to_cpu(sb->bitmap_offset);
		if (__le32_to_cpu(bsb->nodes) > 1)
			info->array.state |= (1 << MD_SB_CLUSTERED);
	} else if (md_feature_any_ppl_on(sb->feature_map)) {
		info->ppl_offset = __le16_to_cpu(sb->ppl.offset);
		info->ppl_size = __le16_to_cpu(sb->ppl.size);
		info->ppl_sector = super_offset + info->ppl_offset;
	}

	info->disk.major = 0;
	info->disk.minor = 0;
	info->disk.number = __le32_to_cpu(sb->dev_number);
	if (__le32_to_cpu(sb->dev_number) >= __le32_to_cpu(sb->max_dev) ||
	    __le32_to_cpu(sb->dev_number) >= MAX_DEVS)
		role = MD_DISK_ROLE_FAULTY;
	else
		role = __le16_to_cpu(sb->dev_roles[__le32_to_cpu(sb->dev_number)]);

	if (info->array.level <= 0)
		data_size = __le64_to_cpu(sb->data_size);
	else
		data_size = __le64_to_cpu(sb->size);
	if (info->data_offset < super_offset) {
		unsigned long long end;
		info->space_before = info->data_offset;
		end = super_offset;

		if (sb->bblog_offset && sb->bblog_size) {
			unsigned long long bboffset = super_offset;
			bboffset += (int32_t)__le32_to_cpu(sb->bblog_offset);
			if (bboffset < end)
				end = bboffset;
		}

		if (super_offset + info->bitmap_offset + info->ppl_offset < end)
			end = super_offset + info->bitmap_offset +
				info->ppl_offset;

		if (info->data_offset + data_size < end)
			info->space_after = end - data_size - info->data_offset;
		else
			info->space_after = 0;
	} else {
		unsigned long long earliest;
		earliest = super_offset + (32+4)*2; /* match kernel */
		if (info->bitmap_offset > 0) {
			unsigned long long bmend = info->bitmap_offset;
			unsigned long long size = calc_bitmap_size(bsb, 4096);
			size /= 512;
			bmend += size;
			if (bmend > earliest)
				earliest = bmend;
		} else if (info->ppl_offset > 0) {
			unsigned long long pplend;

			pplend = info->ppl_offset + info->ppl_size;
			if (pplend > earliest)
				earliest = pplend;
		}
		if (sb->bblog_offset && sb->bblog_size) {
			unsigned long long bbend = super_offset;
			bbend += (int32_t)__le32_to_cpu(sb->bblog_offset);
			bbend += __le16_to_cpu(sb->bblog_size);
			if (bbend > earliest)
				earliest = bbend;
		}
		if (earliest < info->data_offset)
			info->space_before = info->data_offset - earliest;
		else
			info->space_before = 0;
		info->space_after = misc->device_size - data_size -
			info->data_offset;
	}
	if (info->space_before == 0 && info->space_after == 0) {
		/* It will look like we don't support data_offset changes,
		 * be we do - it's just that there is no room.
		 * A change that reduced the number of devices should
		 * still be allowed, so set the otherwise useless value of '1'
		 */
		info->space_after = 1;
	}

	info->disk.raid_disk = -1;
	switch(role) {
	case MD_DISK_ROLE_SPARE:
		/* spare: not active, not sync, not faulty */
		info->disk.state = 0;
		break;
	case MD_DISK_ROLE_FAULTY:
		info->disk.state = (1 << MD_DISK_FAULTY); /* faulty */
		break;
	case MD_DISK_ROLE_JOURNAL:
		info->disk.state = (1 << MD_DISK_JOURNAL);
		info->disk.raid_disk = role;
		/* journal uses all 4kB blocks*/
		info->space_after = (misc->device_size - info->data_offset) % 8;
		break;
	default:
		info->disk.state = 6; /* active and in sync */
		info->disk.raid_disk = role;
	}
	if (sb->devflags & WriteMostly1)
		info->disk.state |= (1 << MD_DISK_WRITEMOSTLY);
	if (sb->devflags & FailFast1)
		info->disk.state |= (1 << MD_DISK_FAILFAST);
	info->events = __le64_to_cpu(sb->events);
	sprintf(info->text_version, "1.%d", st->minor_version);
	info->safe_mode_delay = 200;

	memcpy(info->uuid, sb->set_uuid, 16);

	strncpy(info->name, sb->set_name, 32);
	info->name[32] = 0;

	if ((__le32_to_cpu(sb->feature_map)&MD_FEATURE_REPLACEMENT)) {
		info->disk.state &= ~(1 << MD_DISK_SYNC);
		info->disk.state |=  1 << MD_DISK_REPLACEMENT;
	}

	if (sb->feature_map & __le32_to_cpu(MD_FEATURE_RECOVERY_OFFSET))
		info->recovery_start = __le32_to_cpu(sb->recovery_offset);
	else
		info->recovery_start = MaxSector;

	if (sb->feature_map & __le32_to_cpu(MD_FEATURE_RESHAPE_ACTIVE)) {
		info->reshape_active = 1;
		if ((sb->feature_map & __le32_to_cpu(MD_FEATURE_NEW_OFFSET)) &&
		    sb->new_offset != 0)
			info->reshape_active |= RESHAPE_NO_BACKUP;
		info->reshape_progress = __le64_to_cpu(sb->reshape_position);
		info->new_level = __le32_to_cpu(sb->new_level);
		info->delta_disks = __le32_to_cpu(sb->delta_disks);
		info->new_layout = __le32_to_cpu(sb->new_layout);
		info->new_chunk = __le32_to_cpu(sb->new_chunk)<<9;
		if (info->delta_disks < 0)
			info->array.raid_disks -= info->delta_disks;
	} else
		info->reshape_active = 0;

	info->recovery_blocked = info->reshape_active;

	if (map)
		for (i=0; i<map_disks; i++)
			map[i] = 0;
	for (i = 0; i < __le32_to_cpu(sb->max_dev); i++) {
		role = __le16_to_cpu(sb->dev_roles[i]);
		if (/*role == MD_DISK_ROLE_SPARE || */role < (unsigned) info->array.raid_disks) {
			working++;
			if (map && role < map_disks)
				map[role] = 1;
		}
	}

	info->array.working_disks = working;

	if (sb->feature_map & __le32_to_cpu(MD_FEATURE_JOURNAL)) {
		info->journal_device_required = 1;
		info->consistency_policy = CONSISTENCY_POLICY_JOURNAL;
	} else if (md_feature_any_ppl_on(sb->feature_map)) {
		info->consistency_policy = CONSISTENCY_POLICY_PPL;
	} else if (sb->feature_map & __le32_to_cpu(MD_FEATURE_BITMAP_OFFSET)) {
		info->consistency_policy = CONSISTENCY_POLICY_BITMAP;
	} else if (info->array.level <= 0) {
		info->consistency_policy = CONSISTENCY_POLICY_NONE;
	} else {
		info->consistency_policy = CONSISTENCY_POLICY_RESYNC;
	}

	info->journal_clean = 0;
}

static struct mdinfo *container_content1(struct supertype *st, char *subarray)
{
	struct mdinfo *info;

	if (subarray)
		return NULL;

	info = xmalloc(sizeof(*info));
	getinfo_super1(st, info, NULL);
	return info;
}

static int update_super1(struct supertype *st, struct mdinfo *info,
			 char *update, char *devname, int verbose,
			 int uuid_set, char *homehost)
{
	/* NOTE: for 'assemble' and 'force' we need to return non-zero
	 * if any change was made.  For others, the return value is
	 * ignored.
	 */
	int rv = 0;
	struct mdp_superblock_1 *sb = st->sb;
	bitmap_super_t *bms = (bitmap_super_t*)(((char*)sb) + MAX_SB_SIZE);

	if (strcmp(update, "homehost") == 0 &&
	    homehost) {
		/* Note that 'homehost' is special as it is really
		 * a "name" update.
		 */
		char *c;
		update = "name";
		c = strchr(sb->set_name, ':');
		if (c)
			strncpy(info->name, c+1, 31 - (c-sb->set_name));
		else
			strncpy(info->name, sb->set_name, 32);
		info->name[32] = 0;
	}

	if (strcmp(update, "force-one")==0) {
		/* Not enough devices for a working array,
		 * so bring this one up-to-date
		 */
		if (sb->events != __cpu_to_le64(info->events))
			rv = 1;
		sb->events = __cpu_to_le64(info->events);
	} else if (strcmp(update, "force-array")==0) {
		/* Degraded array and 'force' requests to
		 * maybe need to mark it 'clean'.
		 */
		switch(__le32_to_cpu(sb->level)) {
		case 4:
		case 5:
		case 6:
			/* need to force clean */
			if (sb->resync_offset != MaxSector)
				rv = 1;
			sb->resync_offset = MaxSector;
		}
	} else if (strcmp(update, "assemble")==0) {
		int d = info->disk.number;
		int want;
		if (info->disk.state & (1<<MD_DISK_ACTIVE))
			want = info->disk.raid_disk;
		else if (info->disk.state & (1<<MD_DISK_JOURNAL))
			want = MD_DISK_ROLE_JOURNAL;
		else
			want = MD_DISK_ROLE_SPARE;
		if (sb->dev_roles[d] != __cpu_to_le16(want)) {
			sb->dev_roles[d] = __cpu_to_le16(want);
			rv = 1;
		}
		if (info->reshape_active &&
		    sb->feature_map &
		    __le32_to_cpu(MD_FEATURE_RESHAPE_ACTIVE) &&
		    info->delta_disks >= 0 &&
		    info->reshape_progress <
		    __le64_to_cpu(sb->reshape_position)) {
			sb->reshape_position =
				__cpu_to_le64(info->reshape_progress);
			rv = 1;
		}
		if (info->reshape_active &&
		    sb->feature_map &
		    __le32_to_cpu(MD_FEATURE_RESHAPE_ACTIVE) &&
		    info->delta_disks < 0 &&
		    info->reshape_progress >
		    __le64_to_cpu(sb->reshape_position)) {
			sb->reshape_position =
				__cpu_to_le64(info->reshape_progress);
			rv = 1;
		}
	} else if (strcmp(update, "linear-grow-new") == 0) {
		unsigned int i;
		int fd;
		unsigned int max = __le32_to_cpu(sb->max_dev);

		for (i = 0; i < max; i++)
			if (__le16_to_cpu(sb->dev_roles[i]) >=
			    MD_DISK_ROLE_FAULTY)
				break;
		sb->dev_number = __cpu_to_le32(i);
		info->disk.number = i;
		if (i >= max) {
			sb->max_dev = __cpu_to_le32(max+1);
		}

		random_uuid(sb->device_uuid);

		sb->dev_roles[i] = __cpu_to_le16(info->disk.raid_disk);

		fd = open(devname, O_RDONLY);
		if (fd >= 0) {
			unsigned long long ds;
			get_dev_size(fd, devname, &ds);
			close(fd);
			ds >>= 9;
			if (__le64_to_cpu(sb->super_offset) <
			    __le64_to_cpu(sb->data_offset)) {
				sb->data_size = __cpu_to_le64(
					ds - __le64_to_cpu(sb->data_offset));
			} else {
				ds -= 8*2;
				ds &= ~(unsigned long long)(4*2-1);
				sb->super_offset = __cpu_to_le64(ds);
				sb->data_size = __cpu_to_le64(
					ds - __le64_to_cpu(sb->data_offset));
			}
		}
	} else if (strcmp(update, "linear-grow-update") == 0) {
		int max = __le32_to_cpu(sb->max_dev);
		sb->raid_disks = __cpu_to_le32(info->array.raid_disks);
		if (info->array.raid_disks > max) {
			sb->max_dev = __cpu_to_le32(max+1);
		}
		sb->dev_roles[info->disk.number] =
			__cpu_to_le16(info->disk.raid_disk);
	} else if (strcmp(update, "resync") == 0) {
		/* make sure resync happens */
		sb->resync_offset = 0ULL;
	} else if (strcmp(update, "uuid") == 0) {
		copy_uuid(sb->set_uuid, info->uuid, super1.swapuuid);

		if (__le32_to_cpu(sb->feature_map) & MD_FEATURE_BITMAP_OFFSET)
			memcpy(bms->uuid, sb->set_uuid, 16);
	} else if (strcmp(update, "no-bitmap") == 0) {
		sb->feature_map &= ~__cpu_to_le32(MD_FEATURE_BITMAP_OFFSET);
	} else if (strcmp(update, "bbl") == 0) {
		/* only possible if there is room after the bitmap, or if
		 * there is no bitmap
		 */
		unsigned long long sb_offset = __le64_to_cpu(sb->super_offset);
		unsigned long long data_offset = __le64_to_cpu(sb->data_offset);
		long bitmap_offset = 0;
		long bm_sectors = 0;
		long space;

		if (sb->feature_map & __cpu_to_le32(MD_FEATURE_BITMAP_OFFSET)) {
			bitmap_offset = (long)__le32_to_cpu(sb->bitmap_offset);
			bm_sectors = calc_bitmap_size(bms, 4096) >> 9;
		} else if (md_feature_any_ppl_on(sb->feature_map)) {
			bitmap_offset = (long)__le16_to_cpu(sb->ppl.offset);
			bm_sectors = (long)__le16_to_cpu(sb->ppl.size);
		}

		if (sb_offset < data_offset) {
			/*
			 * 1.1 or 1.2.  Put bbl after bitmap leaving
			 * at least 32K
			 */
			long bb_offset;
			bb_offset = sb_offset + 8;
			if (bm_sectors && bitmap_offset > 0)
				bb_offset = bitmap_offset + bm_sectors;
			while (bb_offset < (long)sb_offset + 8 + 32*2 &&
			       bb_offset + 8+8 <= (long)data_offset)
				/* too close to bitmap, and room to grow */
				bb_offset += 8;
			if (bb_offset + 8 <= (long)data_offset) {
				sb->bblog_size = __cpu_to_le16(8);
				sb->bblog_offset = __cpu_to_le32(bb_offset);
			}
		} else {
			/* 1.0 - Put bbl just before super block */
			if (bm_sectors && bitmap_offset < 0)
				space = -bitmap_offset - bm_sectors;
			else
				space = sb_offset - data_offset -
					__le64_to_cpu(sb->data_size);
			if (space >= 8) {
				sb->bblog_size = __cpu_to_le16(8);
				sb->bblog_offset = __cpu_to_le32((unsigned)-8);
			}
		}
	} else if (strcmp(update, "no-bbl") == 0) {
		if (sb->feature_map & __cpu_to_le32(MD_FEATURE_BAD_BLOCKS))
			pr_err("Cannot remove active bbl from %s\n",devname);
		else {
			sb->bblog_size = 0;
			sb->bblog_shift = 0;
			sb->bblog_offset = 0;
		}
	} else if (strcmp(update, "force-no-bbl") == 0) {
		sb->feature_map &= ~ __cpu_to_le32(MD_FEATURE_BAD_BLOCKS);
		sb->bblog_size = 0;
		sb->bblog_shift = 0;
		sb->bblog_offset = 0;
	} else if (strcmp(update, "ppl") == 0) {
		unsigned long long sb_offset = __le64_to_cpu(sb->super_offset);
		unsigned long long data_offset = __le64_to_cpu(sb->data_offset);
		unsigned long long data_size = __le64_to_cpu(sb->data_size);
		long bb_offset = __le32_to_cpu(sb->bblog_offset);
		int space;
		int offset;

		if (sb->feature_map & __cpu_to_le32(MD_FEATURE_BITMAP_OFFSET)) {
			pr_err("Cannot add PPL to array with bitmap\n");
			return -2;
		}

		if (sb->feature_map & __cpu_to_le32(MD_FEATURE_JOURNAL)) {
			pr_err("Cannot add PPL to array with journal\n");
			return -2;
		}

		if (sb_offset < data_offset) {
			if (bb_offset)
				space = bb_offset - 8;
			else
				space = data_offset - sb_offset - 8;
			offset = 8;
		} else {
			offset = -(sb_offset - data_offset - data_size);
			if (offset < INT16_MIN)
				offset = INT16_MIN;
			space = -(offset - bb_offset);
		}

		if (space < (PPL_HEADER_SIZE >> 9) + 8) {
			pr_err("Not enough space to add ppl\n");
			return -2;
		}

		if (space >= (MULTIPLE_PPL_AREA_SIZE_SUPER1 >> 9)) {
			space = (MULTIPLE_PPL_AREA_SIZE_SUPER1 >> 9);
		} else {
			int optimal_space = choose_ppl_space(
						__le32_to_cpu(sb->chunksize));
			if (space > optimal_space)
				space = optimal_space;
			if (space > UINT16_MAX)
				space = UINT16_MAX;
		}

		sb->ppl.offset = __cpu_to_le16(offset);
		sb->ppl.size = __cpu_to_le16(space);
		sb->feature_map |= __cpu_to_le32(MD_FEATURE_PPL);
	} else if (strcmp(update, "no-ppl") == 0) {
		sb->feature_map &= ~__cpu_to_le32(MD_FEATURE_PPL |
						   MD_FEATURE_MUTLIPLE_PPLS);
	} else if (strcmp(update, "name") == 0) {
		if (info->name[0] == 0)
			sprintf(info->name, "%d", info->array.md_minor);
		memset(sb->set_name, 0, sizeof(sb->set_name));
		if (homehost &&
		    strchr(info->name, ':') == NULL &&
		    strlen(homehost)+1+strlen(info->name) < 32) {
			strcpy(sb->set_name, homehost);
			strcat(sb->set_name, ":");
			strcat(sb->set_name, info->name);
		} else {
			int namelen;

			namelen = min((int)strlen(info->name),
				      (int)sizeof(sb->set_name) - 1);
			memcpy(sb->set_name, info->name, namelen);
			memset(&sb->set_name[namelen], '\0',
			       sizeof(sb->set_name) - namelen);
		}
	} else if (strcmp(update, "devicesize") == 0 &&
		   __le64_to_cpu(sb->super_offset) <
		   __le64_to_cpu(sb->data_offset)) {
		/* set data_size to device size less data_offset */
		struct misc_dev_info *misc = (struct misc_dev_info*)
			(st->sb + MAX_SB_SIZE + BM_SUPER_SIZE);
		sb->data_size = __cpu_to_le64(
			misc->device_size - __le64_to_cpu(sb->data_offset));
	} else if (strncmp(update, "revert-reshape", 14) == 0) {
		rv = -2;
		if (!(sb->feature_map &
		      __cpu_to_le32(MD_FEATURE_RESHAPE_ACTIVE)))
			pr_err("No active reshape to revert on %s\n",
			       devname);
		else {
			__u32 temp;
			unsigned long long reshape_sectors;
			long reshape_chunk;
			rv = 0;
			/* If the reshape hasn't started, just stop it.
			 * It is conceivable that a stripe was modified but
			 * the metadata not updated.  In that case the backup
			 * should have been used to get passed the critical stage.
			 * If that couldn't happen, the "-nobackup" version
			 * will be used.
			 */
			if (strcmp(update, "revert-reshape-nobackup") == 0 &&
			    sb->reshape_position == 0 &&
			    (__le32_to_cpu(sb->delta_disks) > 0 ||
			     (__le32_to_cpu(sb->delta_disks) == 0 &&
			      !(sb->feature_map & __cpu_to_le32(MD_FEATURE_RESHAPE_BACKWARDS))))) {
				sb->feature_map &= ~__cpu_to_le32(MD_FEATURE_RESHAPE_ACTIVE);
				sb->raid_disks = __cpu_to_le32(__le32_to_cpu(sb->raid_disks) -
							       __le32_to_cpu(sb->delta_disks));
				sb->delta_disks = 0;
				goto done;
			}
			/* reshape_position is a little messy.
			 * Its value must be a multiple of the larger
			 * chunk size, and of the "after" data disks.
			 * So when reverting we need to change it to
			 * be a multiple of the new "after" data disks,
			 * which is the old "before".
			 * If it isn't already a multiple of 'before',
			 * the only thing we could do would be
			 * copy some block around on the disks, which
			 * is easy to get wrong.
			 * So we reject a revert-reshape unless the
			 * alignment is good.
			 */
			if (__le32_to_cpu(sb->level) >= 4 &&
			    __le32_to_cpu(sb->level) <= 6) {
				reshape_sectors =
					__le64_to_cpu(sb->reshape_position);
				reshape_chunk = __le32_to_cpu(sb->new_chunk);
				reshape_chunk *= __le32_to_cpu(sb->raid_disks) -
					__le32_to_cpu(sb->delta_disks) -
					(__le32_to_cpu(sb->level)==6 ? 2 : 1);
				if (reshape_sectors % reshape_chunk) {
					pr_err("Reshape position is not suitably aligned.\n");
					pr_err("Try normal assembly and stop again\n");
					return -2;
				}
			}
			sb->raid_disks =
				__cpu_to_le32(__le32_to_cpu(sb->raid_disks) -
					      __le32_to_cpu(sb->delta_disks));
			if (sb->delta_disks == 0)
				sb->feature_map ^= __cpu_to_le32(MD_FEATURE_RESHAPE_BACKWARDS);
			else
				sb->delta_disks = __cpu_to_le32(-__le32_to_cpu(sb->delta_disks));

			temp = sb->new_layout;
			sb->new_layout = sb->layout;
			sb->layout = temp;

			temp = sb->new_chunk;
			sb->new_chunk = sb->chunksize;
			sb->chunksize = temp;

			if (sb->feature_map &
			    __cpu_to_le32(MD_FEATURE_NEW_OFFSET)) {
				long offset_delta =
					(int32_t)__le32_to_cpu(sb->new_offset);
				sb->data_offset = __cpu_to_le64(__le64_to_cpu(sb->data_offset) + offset_delta);
				sb->new_offset = __cpu_to_le32(-offset_delta);
				sb->data_size = __cpu_to_le64(__le64_to_cpu(sb->data_size) - offset_delta);
			}
		done:;
		}
	} else if (strcmp(update, "_reshape_progress") == 0)
		sb->reshape_position = __cpu_to_le64(info->reshape_progress);
	else if (strcmp(update, "writemostly") == 0)
		sb->devflags |= WriteMostly1;
	else if (strcmp(update, "readwrite") == 0)
		sb->devflags &= ~WriteMostly1;
	else if (strcmp(update, "failfast") == 0)
		sb->devflags |= FailFast1;
	else if (strcmp(update, "nofailfast") == 0)
		sb->devflags &= ~FailFast1;
	else
		rv = -1;

	sb->sb_csum = calc_sb_1_csum(sb);

	return rv;
}

static int init_super1(struct supertype *st, mdu_array_info_t *info,
		       struct shape *s, char *name, char *homehost,
		       int *uuid, unsigned long long data_offset)
{
	struct mdp_superblock_1 *sb;
	int spares;
	char defname[10];
	int sbsize;

	if (posix_memalign((void**)&sb, 4096, SUPER1_SIZE) != 0) {
		pr_err("could not allocate superblock\n");
		return 0;
	}
	memset(sb, 0, SUPER1_SIZE);

	st->sb = sb;
	if (info == NULL) {
		/* zeroing superblock */
		return 0;
	}

	spares = info->working_disks - info->active_disks;
	if (info->raid_disks + spares  > MAX_DEVS) {
		pr_err("too many devices requested: %d+%d > %d\n",
			info->raid_disks , spares, MAX_DEVS);
		return 0;
	}

	sb->magic = __cpu_to_le32(MD_SB_MAGIC);
	sb->major_version = __cpu_to_le32(1);
	sb->feature_map = 0;
	sb->pad0 = 0;

	if (uuid)
		copy_uuid(sb->set_uuid, uuid, super1.swapuuid);
	else
		random_uuid(sb->set_uuid);;

	if (name == NULL || *name == 0) {
		sprintf(defname, "%d", info->md_minor);
		name = defname;
	}
	if (homehost &&
	    strchr(name, ':')== NULL &&
	    strlen(homehost)+1+strlen(name) < 32) {
		strcpy(sb->set_name, homehost);
		strcat(sb->set_name, ":");
		strcat(sb->set_name, name);
	} else {
		int namelen;

		namelen = min((int)strlen(name),
			      (int)sizeof(sb->set_name) - 1);
		memcpy(sb->set_name, name, namelen);
		memset(&sb->set_name[namelen], '\0',
		       sizeof(sb->set_name) - namelen);
	}

	sb->ctime = __cpu_to_le64((unsigned long long)time(0));
	sb->level = __cpu_to_le32(info->level);
	sb->layout = __cpu_to_le32(info->layout);
	sb->size = __cpu_to_le64(s->size*2ULL);
	sb->chunksize = __cpu_to_le32(info->chunk_size>>9);
	sb->raid_disks = __cpu_to_le32(info->raid_disks);

	sb->data_offset = __cpu_to_le64(data_offset);
	sb->data_size = __cpu_to_le64(0);
	sb->super_offset = __cpu_to_le64(0);
	sb->recovery_offset = __cpu_to_le64(0);

	sb->utime = sb->ctime;
	sb->events = __cpu_to_le64(1);
	if (info->state & (1<<MD_SB_CLEAN))
		sb->resync_offset = MaxSector;
	else
		sb->resync_offset = 0;
	sbsize = sizeof(struct mdp_superblock_1) +
		2 * (info->raid_disks + spares);
	sbsize = ROUND_UP(sbsize, 512);
	sb->max_dev =
		__cpu_to_le32((sbsize - sizeof(struct mdp_superblock_1)) / 2);

	memset(sb->dev_roles, 0xff,
	       MAX_SB_SIZE - sizeof(struct mdp_superblock_1));

	if (s->consistency_policy == CONSISTENCY_POLICY_PPL)
		sb->feature_map |= __cpu_to_le32(MD_FEATURE_PPL);

	return 1;
}

struct devinfo {
	int fd;
	char *devname;
	long long data_offset;
	mdu_disk_info_t disk;
	struct devinfo *next;
};

/* Add a device to the superblock being created */
static int add_to_super1(struct supertype *st, mdu_disk_info_t *dk,
			 int fd, char *devname, unsigned long long data_offset)
{
	struct mdp_superblock_1 *sb = st->sb;
	__u16 *rp = sb->dev_roles + dk->number;
	struct devinfo *di, **dip;
	int dk_state;

	dk_state = dk->state & ~(1<<MD_DISK_FAILFAST);
	if ((dk_state & (1<<MD_DISK_ACTIVE)) &&
	    (dk_state & (1<<MD_DISK_SYNC)))/* active, sync */
		*rp = __cpu_to_le16(dk->raid_disk);
	else if (dk_state & (1<<MD_DISK_JOURNAL))
                *rp = MD_DISK_ROLE_JOURNAL;
	else if ((dk_state & ~(1<<MD_DISK_ACTIVE)) == 0)
		/* active or idle -> spare */
		*rp = MD_DISK_ROLE_SPARE;
	else
		*rp = MD_DISK_ROLE_FAULTY;

	if (dk->number >= (int)__le32_to_cpu(sb->max_dev) &&
	    __le32_to_cpu(sb->max_dev) < MAX_DEVS)
		sb->max_dev = __cpu_to_le32(dk->number+1);

	sb->dev_number = __cpu_to_le32(dk->number);
	sb->devflags = 0; /* don't copy another disks flags */
	sb->sb_csum = calc_sb_1_csum(sb);

	dip = (struct devinfo **)&st->info;
	while (*dip)
		dip = &(*dip)->next;
	di = xmalloc(sizeof(struct devinfo));
	di->fd = fd;
	di->devname = devname;
	di->disk = *dk;
	di->data_offset = data_offset;
	di->next = NULL;
	*dip = di;

	return 0;
}

static int locate_bitmap1(struct supertype *st, int fd, int node_num);

static int store_super1(struct supertype *st, int fd)
{
	struct mdp_superblock_1 *sb = st->sb;
	unsigned long long sb_offset;
	struct align_fd afd;
	int sbsize;
	unsigned long long dsize;

	if (!get_dev_size(fd, NULL, &dsize))
		return 1;

	dsize >>= 9;

	if (dsize < 24)
		return 2;

	init_afd(&afd, fd);

	/*
	 * Calculate the position of the superblock.
	 * It is always aligned to a 4K boundary and
	 * depending on minor_version, it can be:
	 * 0: At least 8K, but less than 12K, from end of device
	 * 1: At start of device
	 * 2: 4K from start of device.
	 */
	switch(st->minor_version) {
	case 0:
		sb_offset = dsize;
		sb_offset -= 8*2;
		sb_offset &= ~(4*2-1);
		break;
	case 1:
		sb_offset = 0;
		break;
	case 2:
		sb_offset = 4*2;
		break;
	default:
		return -EINVAL;
	}

	if (sb_offset != __le64_to_cpu(sb->super_offset) &&
	    0 != __le64_to_cpu(sb->super_offset)
		) {
		pr_err("internal error - sb_offset is wrong\n");
		abort();
	}

	if (lseek64(fd, sb_offset << 9, 0)< 0LL)
		return 3;

	sbsize = ROUND_UP(sizeof(*sb) + 2 * __le32_to_cpu(sb->max_dev), 512);

	if (awrite(&afd, sb, sbsize) != sbsize)
		return 4;

	if (sb->feature_map & __cpu_to_le32(MD_FEATURE_BITMAP_OFFSET)) {
		struct bitmap_super_s *bm = (struct bitmap_super_s*)
			(((char*)sb)+MAX_SB_SIZE);
		if (__le32_to_cpu(bm->magic) == BITMAP_MAGIC) {
			locate_bitmap1(st, fd, 0);
			if (awrite(&afd, bm, sizeof(*bm)) != sizeof(*bm))
				return 5;
		}
	}
	fsync(fd);

	return 0;
}

static int load_super1(struct supertype *st, int fd, char *devname);

static unsigned long choose_bm_space(unsigned long devsize)
{
	/* if the device is bigger than 8Gig, save 64k for bitmap usage,
	 * if bigger than 200Gig, save 128k
	 * NOTE: result must be multiple of 4K else bad things happen
	 * on 4K-sector devices.
	 */
	if (devsize < 64*2)
		return 0;
	if (devsize - 64*2 >= 200*1024*1024*2)
		return 128*2;
	if (devsize - 4*2 > 8*1024*1024*2)
		return 64*2;
	return 4*2;
}

static void free_super1(struct supertype *st);

__u32 crc32c_le(__u32 crc, unsigned char const *p, size_t len);

static int write_init_ppl1(struct supertype *st, struct mdinfo *info, int fd)
{
	struct mdp_superblock_1 *sb = st->sb;
	void *buf;
	struct ppl_header *ppl_hdr;
	int ret;

	/* first clear entire ppl space */
	ret = zero_disk_range(fd, info->ppl_sector, info->ppl_size);
	if (ret)
		return ret;

	ret = posix_memalign(&buf, 4096, PPL_HEADER_SIZE);
	if (ret) {
		pr_err("Failed to allocate PPL header buffer\n");
		return ret;
	}

	memset(buf, 0, PPL_HEADER_SIZE);
	ppl_hdr = buf;
	memset(ppl_hdr->reserved, 0xff, PPL_HDR_RESERVED);
	ppl_hdr->signature = __cpu_to_le32(~crc32c_le(~0, sb->set_uuid,
						      sizeof(sb->set_uuid)));
	ppl_hdr->checksum = __cpu_to_le32(~crc32c_le(~0, buf, PPL_HEADER_SIZE));

	if (lseek64(fd, info->ppl_sector * 512, SEEK_SET) < 0) {
		ret = errno;
		perror("Failed to seek to PPL header location");
	}

	if (!ret && write(fd, buf, PPL_HEADER_SIZE) != PPL_HEADER_SIZE) {
		ret = errno;
		perror("Write PPL header failed");
	}

	if (!ret)
		fsync(fd);

	free(buf);
	return ret;
}

#define META_BLOCK_SIZE 4096

static int write_empty_r5l_meta_block(struct supertype *st, int fd)
{
	struct r5l_meta_block *mb;
	struct mdp_superblock_1 *sb = st->sb;
	struct align_fd afd;
	__u32 crc;

	init_afd(&afd, fd);

	if (posix_memalign((void**)&mb, 4096, META_BLOCK_SIZE) != 0) {
		pr_err("Could not allocate memory for the meta block.\n");
		return 1;
	}

	memset(mb, 0, META_BLOCK_SIZE);

	mb->magic = __cpu_to_le32(R5LOG_MAGIC);
	mb->version = R5LOG_VERSION;
	mb->meta_size = __cpu_to_le32(sizeof(struct r5l_meta_block));
	mb->seq = __cpu_to_le64(random32());
	mb->position = __cpu_to_le64(0);

	crc = crc32c_le(0xffffffff, sb->set_uuid, sizeof(sb->set_uuid));
	crc = crc32c_le(crc, (void *)mb, META_BLOCK_SIZE);
	mb->checksum = crc;

	if (lseek64(fd, __le64_to_cpu(sb->data_offset) * 512, 0) < 0LL) {
		pr_err("cannot seek to offset of the meta block\n");
		goto fail_to_write;
	}

	if (awrite(&afd, mb, META_BLOCK_SIZE) != META_BLOCK_SIZE) {
		pr_err("failed to store write the meta block \n");
		goto fail_to_write;
	}
	fsync(fd);

	free(mb);
	return 0;

fail_to_write:
	free(mb);
	return 1;
}

static int write_init_super1(struct supertype *st)
{
	struct mdp_superblock_1 *sb = st->sb;
	struct supertype *refst;
	int rv = 0;
	unsigned long long bm_space;
	struct devinfo *di;
	unsigned long long dsize, array_size;
	unsigned long long sb_offset;
	unsigned long long data_offset;
	long bm_offset;

	for (di = st->info; di; di = di->next) {
		if (di->disk.state & (1 << MD_DISK_JOURNAL))
			sb->feature_map |= __cpu_to_le32(MD_FEATURE_JOURNAL);
	}

	for (di = st->info; di; di = di->next) {
		if (di->disk.state & (1 << MD_DISK_FAULTY))
			continue;
		if (di->fd < 0)
			continue;

		while (Kill(di->devname, NULL, 0, -1, 1) == 0)
			;

		sb->dev_number = __cpu_to_le32(di->disk.number);
		if (di->disk.state & (1<<MD_DISK_WRITEMOSTLY))
			sb->devflags |= WriteMostly1;
		else
			sb->devflags &= ~WriteMostly1;
		if (di->disk.state & (1<<MD_DISK_FAILFAST))
			sb->devflags |= FailFast1;
		else
			sb->devflags &= ~FailFast1;

		random_uuid(sb->device_uuid);

		if (!(di->disk.state & (1<<MD_DISK_JOURNAL)))
			sb->events = 0;

		refst = dup_super(st);
		if (load_super1(refst, di->fd, NULL)==0) {
			struct mdp_superblock_1 *refsb = refst->sb;

			memcpy(sb->device_uuid, refsb->device_uuid, 16);
			if (memcmp(sb->set_uuid, refsb->set_uuid, 16)==0) {
				/* same array, so preserve events and
				 * dev_number */
				sb->events = refsb->events;
				/* bugs in 2.6.17 and earlier mean the
				 * dev_number chosen in Manage must be preserved
				 */
				if (get_linux_version() >= 2006018)
					sb->dev_number = refsb->dev_number;
			}
			free_super1(refst);
		}
		free(refst);

		if (!get_dev_size(di->fd, NULL, &dsize)) {
			rv = 1;
			goto error_out;
		}
		dsize >>= 9;

		if (dsize < 24) {
			close(di->fd);
			rv = 2;
			goto error_out;
		}

		/*
		 * Calculate the position of the superblock.
		 * It is always aligned to a 4K boundary and
		 * depending on minor_version, it can be:
		 * 0: At least 8K, but less than 12K, from end of device
		 * 1: At start of device
		 * 2: 4K from start of device.
		 * data_offset has already been set.
		 */
		array_size = __le64_to_cpu(sb->size);

		/* work out how much space we left for a bitmap */
		if (sb->feature_map & __cpu_to_le32(MD_FEATURE_BITMAP_OFFSET)) {
			bitmap_super_t *bms = (bitmap_super_t *)
					(((char *)sb) + MAX_SB_SIZE);
			bm_space = calc_bitmap_size(bms, 4096) >> 9;
			bm_offset = (long)__le32_to_cpu(sb->bitmap_offset);
		} else if (md_feature_any_ppl_on(sb->feature_map)) {
			bm_space = MULTIPLE_PPL_AREA_SIZE_SUPER1 >> 9;
			if (st->minor_version == 0)
				bm_offset = -bm_space - 8;
			else
				bm_offset = 8;
			sb->ppl.offset = __cpu_to_le16(bm_offset);
			sb->ppl.size = __cpu_to_le16(bm_space);
		} else {
			bm_space = choose_bm_space(array_size);
			bm_offset = 8;
		}

		data_offset = di->data_offset;
		if (data_offset == INVALID_SECTORS)
			data_offset = st->data_offset;
		switch(st->minor_version) {
		case 0:
			/* Add 8 sectors for bad block log */
			bm_space += 8;
			if (data_offset == INVALID_SECTORS)
				data_offset = 0;
			sb_offset = dsize;
			sb_offset -= 8*2;
			sb_offset &= ~(4*2-1);
			sb->data_offset = __cpu_to_le64(data_offset);
			sb->super_offset = __cpu_to_le64(sb_offset);
			if (sb_offset < array_size + bm_space)
				bm_space = sb_offset - array_size;
			sb->data_size = __cpu_to_le64(sb_offset - bm_space);
			if (bm_space >= 8) {
				sb->bblog_size = __cpu_to_le16(8);
				sb->bblog_offset = __cpu_to_le32((unsigned)-8);
			}
			break;
		case 1:
		case 2:
			sb_offset = st->minor_version == 2 ? 8 : 0;
			sb->super_offset = __cpu_to_le64(sb_offset);
			if (data_offset == INVALID_SECTORS)
				data_offset = sb_offset + 16;

			sb->data_offset = __cpu_to_le64(data_offset);
			sb->data_size = __cpu_to_le64(dsize - data_offset);
			if (data_offset >= sb_offset+bm_offset+bm_space+8) {
				sb->bblog_size = __cpu_to_le16(8);
				sb->bblog_offset = __cpu_to_le32(bm_offset +
								 bm_space);
			} else if (data_offset >= sb_offset + 16) {
				sb->bblog_size = __cpu_to_le16(8);
				/* '8' sectors for the bblog, and 'sb_offset'
				 * because we want offset from superblock, not
				 * start of device.
				 */
				sb->bblog_offset = __cpu_to_le32(data_offset -
								 8 - sb_offset);
			}
			break;
		default:
			pr_err("Failed to write invalid metadata format 1.%i to %s\n",
			       st->minor_version, di->devname);
			rv = -EINVAL;
			goto out;
		}
		/*
		 * Disable badblock log on clusters, or when
		 * explicitly requested
		 */
		if (st->nodes > 0 || conf_get_create_info()->bblist == 0) {
			sb->bblog_size = 0;
			sb->bblog_offset = 0;
		}

		sb->sb_csum = calc_sb_1_csum(sb);
		rv = store_super1(st, di->fd);

		if (rv == 0 && (di->disk.state & (1 << MD_DISK_JOURNAL))) {
			rv = write_empty_r5l_meta_block(st, di->fd);
			if (rv)
				goto error_out;
		}

		if (rv == 0 &&
		    (__le32_to_cpu(sb->feature_map) &
		     MD_FEATURE_BITMAP_OFFSET)) {
			rv = st->ss->write_bitmap(st, di->fd, NodeNumUpdate);
		} else if (rv == 0 &&
		    md_feature_any_ppl_on(sb->feature_map)) {
			struct mdinfo info;

			st->ss->getinfo_super(st, &info, NULL);
			rv = st->ss->write_init_ppl(st, &info, di->fd);
		}

		close(di->fd);
		di->fd = -1;
		if (rv)
			goto error_out;
	}
error_out:
	if (rv)
		pr_err("Failed to write metadata to %s\n", di->devname);
out:
	return rv;
}

static int compare_super1(struct supertype *st, struct supertype *tst)
{
	/*
	 * return:
	 *  0 same, or first was empty, and second was copied
	 *  1 second had wrong number
	 *  2 wrong uuid
	 *  3 wrong other info
	 */
	struct mdp_superblock_1 *first = st->sb;
	struct mdp_superblock_1 *second = tst->sb;

	if (second->magic != __cpu_to_le32(MD_SB_MAGIC))
		return 1;
	if (second->major_version != __cpu_to_le32(1))
		return 1;

	if (!first) {
		if (posix_memalign((void**)&first, 4096, SUPER1_SIZE) != 0) {
			pr_err("could not allocate superblock\n");
			return 1;
		}
		memcpy(first, second, SUPER1_SIZE);
		st->sb = first;
		return 0;
	}
	if (memcmp(first->set_uuid, second->set_uuid, 16)!= 0)
		return 2;

	if (first->ctime      != second->ctime     ||
	    first->level      != second->level     ||
	    first->layout     != second->layout    ||
	    first->size       != second->size      ||
	    first->chunksize  != second->chunksize ||
	    first->raid_disks != second->raid_disks)
		return 3;
	return 0;
}

static int load_super1(struct supertype *st, int fd, char *devname)
{
	unsigned long long dsize;
	unsigned long long sb_offset;
	struct mdp_superblock_1 *super;
	int uuid[4];
	struct bitmap_super_s *bsb;
	struct misc_dev_info *misc;
	struct align_fd afd;

	free_super1(st);

	init_afd(&afd, fd);

	if (st->ss == NULL || st->minor_version == -1) {
		int bestvers = -1;
		struct supertype tst;
		__u64 bestctime = 0;
		/* guess... choose latest ctime */
		memset(&tst, 0, sizeof(tst));
		tst.ss = &super1;
		for (tst.minor_version = 0; tst.minor_version <= 2;
		     tst.minor_version++) {
			switch(load_super1(&tst, fd, devname)) {
			case 0: super = tst.sb;
				if (bestvers == -1 ||
				    bestctime < __le64_to_cpu(super->ctime)) {
					bestvers = tst.minor_version;
					bestctime = __le64_to_cpu(super->ctime);
				}
				free(super);
				tst.sb = NULL;
				break;
			case 1: return 1; /*bad device */
			case 2: break; /* bad, try next */
			}
		}
		if (bestvers != -1) {
			int rv;
			tst.minor_version = bestvers;
			tst.ss = &super1;
			tst.max_devs = MAX_DEVS;
			rv = load_super1(&tst, fd, devname);
			if (rv == 0)
				*st = tst;
			return rv;
		}
		return 2;
	}
	if (!get_dev_size(fd, devname, &dsize))
		return 1;
	dsize >>= 9;

	if (dsize < 24) {
		if (devname)
			pr_err("%s is too small for md: size is %llu sectors.\n",
				devname, dsize);
		return 1;
	}

	/*
	 * Calculate the position of the superblock.
	 * It is always aligned to a 4K boundary and
	 * depending on minor_version, it can be:
	 * 0: At least 8K, but less than 12K, from end of device
	 * 1: At start of device
	 * 2: 4K from start of device.
	 */
	switch(st->minor_version) {
	case 0:
		sb_offset = dsize;
		sb_offset -= 8*2;
		sb_offset &= ~(4*2-1);
		break;
	case 1:
		sb_offset = 0;
		break;
	case 2:
		sb_offset = 4*2;
		break;
	default:
		return -EINVAL;
	}

	if (lseek64(fd, sb_offset << 9, 0)< 0LL) {
		if (devname)
			pr_err("Cannot seek to superblock on %s: %s\n",
				devname, strerror(errno));
		return 1;
	}

	if (posix_memalign((void**)&super, 4096, SUPER1_SIZE) != 0) {
		pr_err("could not allocate superblock\n");
		return 1;
	}

	memset(super, 0, SUPER1_SIZE);

	if (aread(&afd, super, MAX_SB_SIZE) != MAX_SB_SIZE) {
		if (devname)
			pr_err("Cannot read superblock on %s\n",
				devname);
		free(super);
		return 1;
	}

	if (__le32_to_cpu(super->magic) != MD_SB_MAGIC) {
		if (devname)
			pr_err("No super block found on %s (Expected magic %08x, got %08x)\n",
				devname, MD_SB_MAGIC,
			       __le32_to_cpu(super->magic));
		free(super);
		return 2;
	}

	if (__le32_to_cpu(super->major_version) != 1) {
		if (devname)
			pr_err("Cannot interpret superblock on %s - version is %d\n",
				devname, __le32_to_cpu(super->major_version));
		free(super);
		return 2;
	}
	if (__le64_to_cpu(super->super_offset) != sb_offset) {
		if (devname)
			pr_err("No superblock found on %s (super_offset is wrong)\n",
				devname);
		free(super);
		return 2;
	}
	st->sb = super;

	bsb = (struct bitmap_super_s *)(((char*)super)+MAX_SB_SIZE);

	misc = (struct misc_dev_info*)
	  (((char*)super)+MAX_SB_SIZE+BM_SUPER_SIZE);
	misc->device_size = dsize;
	if (st->data_offset == INVALID_SECTORS)
		st->data_offset = __le64_to_cpu(super->data_offset);

	/* Now check on the bitmap superblock */
	if ((__le32_to_cpu(super->feature_map)&MD_FEATURE_BITMAP_OFFSET) == 0)
		return 0;
	/* Read the bitmap superblock and make sure it looks
	 * valid.  If it doesn't clear the bit.  An --assemble --force
	 * should get that written out.
	 */
	locate_bitmap1(st, fd, 0);
	if (aread(&afd, bsb, 512) != 512)
		goto no_bitmap;

	uuid_from_super1(st, uuid);
	if (__le32_to_cpu(bsb->magic) != BITMAP_MAGIC ||
	    memcmp(bsb->uuid, uuid, 16) != 0)
		goto no_bitmap;
	return 0;

 no_bitmap:
	super->feature_map = __cpu_to_le32(__le32_to_cpu(super->feature_map)
					   & ~MD_FEATURE_BITMAP_OFFSET);
	return 0;
}

static struct supertype *match_metadata_desc1(char *arg)
{
	struct supertype *st = xcalloc(1, sizeof(*st));

	st->container_devnm[0] = 0;
	st->ss = &super1;
	st->max_devs = MAX_DEVS;
	st->sb = NULL;
	st->data_offset = INVALID_SECTORS;
	/* leading zeros can be safely ignored.  --detail generates them. */
	while (*arg == '0')
		arg++;
	if (strcmp(arg, "1.0") == 0 || strcmp(arg, "1.00") == 0) {
		st->minor_version = 0;
		return st;
	}
	if (strcmp(arg, "1.1") == 0 || strcmp(arg, "1.01") == 0
		) {
		st->minor_version = 1;
		return st;
	}
	if (strcmp(arg, "1.2") == 0 ||
#ifndef DEFAULT_OLD_METADATA /* ifdef in super0.c */
	    strcmp(arg, "default") == 0 ||
#endif /* DEFAULT_OLD_METADATA */
	    strcmp(arg, "1.02") == 0) {
		st->minor_version = 2;
		return st;
	}
	if (strcmp(arg, "1") == 0 || strcmp(arg, "default") == 0) {
		st->minor_version = -1;
		return st;
	}

	free(st);
	return NULL;
}

/* find available size on device with this devsize, using
 * superblock type st, and reserving 'reserve' sectors for
 * a possible bitmap
 */
static __u64 avail_size1(struct supertype *st, __u64 devsize,
			 unsigned long long data_offset)
{
	struct mdp_superblock_1 *super = st->sb;
	int bmspace = 0;
	int bbspace = 0;
	if (devsize < 24)
		return 0;

	if (__le32_to_cpu(super->feature_map) & MD_FEATURE_BITMAP_OFFSET) {
		/* hot-add. allow for actual size of bitmap */
		struct bitmap_super_s *bsb;
		bsb = (struct bitmap_super_s *)(((char*)super)+MAX_SB_SIZE);
		bmspace = calc_bitmap_size(bsb, 4096) >> 9;
	} else if (md_feature_any_ppl_on(super->feature_map)) {
		bmspace = __le16_to_cpu(super->ppl.size);
	}

	/* Allow space for bad block log */
	if (super->bblog_size)
		bbspace = __le16_to_cpu(super->bblog_size);

	if (st->minor_version < 0)
		/* not specified, so time to set default */
		st->minor_version = 2;

	if (data_offset == INVALID_SECTORS)
		data_offset = st->data_offset;

	if (data_offset != INVALID_SECTORS)
		switch(st->minor_version) {
		case 0:
			return devsize - data_offset - 8*2 - bbspace;
		case 1:
		case 2:
			return devsize - data_offset;
		default:
			return 0;
		}

	devsize -= bmspace;

	switch(st->minor_version) {
	case 0:
		/* at end */
		return ((devsize - 8*2 - bbspace ) & ~(4*2-1));
	case 1:
		/* at start, 4K for superblock and possible bitmap */
		return devsize - 4*2 - bbspace;
	case 2:
		/* 4k from start, 4K for superblock and possible bitmap */
		return devsize - (4+4)*2 - bbspace;
	}
	return 0;
}

static int
add_internal_bitmap1(struct supertype *st,
		     int *chunkp, int delay, int write_behind,
		     unsigned long long size,
		     int may_change, int major)
{
	/*
	 * If not may_change, then this is a 'Grow' without sysfs support for
	 * bitmaps, and the bitmap must fit after the superblock at 1K offset.
	 * If may_change, then this is create or a Grow with sysfs support,
	 * and we can put the bitmap wherever we like.
	 *
	 * size is in sectors,  chunk is in bytes !!!
	 */

	unsigned long long bits;
	unsigned long long max_bits;
	unsigned long long min_chunk;
	long offset;
	long bbl_offset, bbl_size;
	unsigned long long chunk = *chunkp;
	int room = 0;
	int creating = 0;
	int len;
	struct mdp_superblock_1 *sb = st->sb;
	bitmap_super_t *bms = (bitmap_super_t*)(((char*)sb) + MAX_SB_SIZE);
	int uuid[4];

	if (__le64_to_cpu(sb->data_size) == 0)
		/*
		 * Must be creating the array, else data_size
		 * would be non-zero
		 */
		creating = 1;
	switch(st->minor_version) {
	case 0:
		/*
		 * either 3K after the superblock (when hot-add),
		 * or some amount of space before.
		 */
		if (creating) {
			/*
			 * We are creating array, so we *know* how much room has
			 * been left.
			 */
			offset = 0;
			bbl_size = 8;
			room =
			  choose_bm_space(__le64_to_cpu(sb->size)) + bbl_size;
		} else {
			room = __le64_to_cpu(sb->super_offset)
				- __le64_to_cpu(sb->data_offset)
				- __le64_to_cpu(sb->data_size);
			bbl_size = __le16_to_cpu(sb->bblog_size);
			if (bbl_size < 8)
				bbl_size = 8;
			bbl_offset = (__s32)__le32_to_cpu(sb->bblog_offset);
			if (bbl_size < -bbl_offset)
				bbl_size = -bbl_offset;

			if (!may_change ||
			    (room < 3*2 && __le32_to_cpu(sb->max_dev) <= 384)) {
				room = 3*2;
				offset = 1*2;
				bbl_size = 0;
			} else {
				offset = 0; /* means movable offset */
			}
		}
		break;
	case 1:
	case 2: /* between superblock and data */
		if (creating) {
			offset = 4*2;
			bbl_size = 8;
			room =
			  choose_bm_space(__le64_to_cpu(sb->size)) + bbl_size;
		} else {
			room = __le64_to_cpu(sb->data_offset)
				- __le64_to_cpu(sb->super_offset);
			bbl_size = __le16_to_cpu(sb->bblog_size);
			if (bbl_size)
				room =
				  __le32_to_cpu(sb->bblog_offset) + bbl_size;
			else
				bbl_size = 8;

			if (!may_change) {
				room -= 2; /* Leave 1K for superblock */
				offset = 2;
				bbl_size = 0;
			} else {
				room -= 4*2; /* leave 4K for superblock */
				offset = 4*2;
			}
		}
		break;
	default:
		return -ENOSPC;
	}

	room -= bbl_size;
	if (chunk == UnSet && room > 128*2)
		/* Limit to 128K of bitmap when chunk size not requested */
		room = 128*2;

	if (room <= 1)
		/* No room for a bitmap */
		return -ENOSPC;

	max_bits = (room * 512 - sizeof(bitmap_super_t)) * 8;

	min_chunk = 4096; /* sub-page chunks don't work yet.. */
	bits = (size*512)/min_chunk +1;
	while (bits > max_bits) {
		min_chunk *= 2;
		bits = (bits+1)/2;
	}
	if (chunk == UnSet) {
		/* For practical purpose, 64Meg is a good
		 * default chunk size for internal bitmaps.
		 */
		chunk = min_chunk;
		if (chunk < 64*1024*1024)
			chunk = 64*1024*1024;
	} else if (chunk < min_chunk)
		return -EINVAL; /* chunk size too small */
	if (chunk == 0) /* rounding problem */
		return -EINVAL;

	if (offset == 0) {
		/* start bitmap on a 4K boundary with enough space for
		 * the bitmap
		 */
		bits = (size*512) / chunk + 1;
		room = ((bits+7)/8 + sizeof(bitmap_super_t) +4095)/4096;
		room *= 8; /* convert 4K blocks to sectors */
		offset = -room - bbl_size;
	}

	sb->bitmap_offset = (int32_t)__cpu_to_le32(offset);

	sb->feature_map = __cpu_to_le32(__le32_to_cpu(sb->feature_map) |
					MD_FEATURE_BITMAP_OFFSET);
	memset(bms, 0, sizeof(*bms));
	bms->magic = __cpu_to_le32(BITMAP_MAGIC);
	bms->version = __cpu_to_le32(major);
	uuid_from_super1(st, uuid);
	memcpy(bms->uuid, uuid, 16);
	bms->chunksize = __cpu_to_le32(chunk);
	bms->daemon_sleep = __cpu_to_le32(delay);
	bms->sync_size = __cpu_to_le64(size);
	bms->write_behind = __cpu_to_le32(write_behind);
	bms->nodes = __cpu_to_le32(st->nodes);
	if (st->nodes)
		sb->feature_map = __cpu_to_le32(__le32_to_cpu(sb->feature_map) |
						MD_FEATURE_BITMAP_VERSIONED);
	if (st->cluster_name) {
		len = sizeof(bms->cluster_name);
		strncpy((char *)bms->cluster_name, st->cluster_name, len);
		bms->cluster_name[len - 1] = '\0';
	}

	*chunkp = chunk;
	return 0;
}

static int locate_bitmap1(struct supertype *st, int fd, int node_num)
{
	unsigned long long offset;
	struct mdp_superblock_1 *sb;
	int mustfree = 0;
	int ret;

	if (!st->sb) {
		if (st->ss->load_super(st, fd, NULL))
			return -1; /* no error I hope... */
		mustfree = 1;
	}
	sb = st->sb;

	if ((__le32_to_cpu(sb->feature_map) & MD_FEATURE_BITMAP_OFFSET))
		ret = 0;
	else
		ret = -1;
	offset = __le64_to_cpu(sb->super_offset);
	offset += (int32_t) __le32_to_cpu(sb->bitmap_offset) * (node_num + 1);
	if (mustfree)
		free(sb);
	lseek64(fd, offset<<9, 0);
	return ret;
}

static int write_bitmap1(struct supertype *st, int fd, enum bitmap_update update)
{
	struct mdp_superblock_1 *sb = st->sb;
	bitmap_super_t *bms = (bitmap_super_t*)(((char*)sb)+MAX_SB_SIZE);
	int rv = 0;
	void *buf;
	int towrite, n, len;
	struct align_fd afd;
	unsigned int i = 0;
	unsigned long long total_bm_space, bm_space_per_node;

	switch (update) {
	case NameUpdate:
		/* update cluster name */
		if (st->cluster_name) {
			len = sizeof(bms->cluster_name);
			memset((char *)bms->cluster_name, 0, len);
			strncpy((char *)bms->cluster_name,
				st->cluster_name, len);
			bms->cluster_name[len - 1] = '\0';
		}
		break;
	case NodeNumUpdate:
		/* cluster md only supports superblock 1.2 now */
		if (st->minor_version != 2 &&
		    bms->version == BITMAP_MAJOR_CLUSTERED) {
			pr_err("Warning: cluster md only works with superblock 1.2\n");
			return -EINVAL;
		}

		if (bms->version == BITMAP_MAJOR_CLUSTERED) {
			if (__cpu_to_le32(st->nodes) < bms->nodes) {
				/*
				 * Since the nodes num is not increased, no
				 * need to check the space enough or not,
				 * just update bms->nodes
				 */
				bms->nodes = __cpu_to_le32(st->nodes);
				break;
			}
		} else {
			/*
			 * no need to change bms->nodes for other
			 * bitmap types
			 */
			if (st->nodes)
				pr_err("Warning: --nodes option is only suitable for clustered bitmap\n");
			break;
		}

		/*
		 * Each node has an independent bitmap, it is necessary to
		 * calculate the space is enough or not, first get how many
		 * bytes for the total bitmap
		 */
		bm_space_per_node = calc_bitmap_size(bms, 4096);

		total_bm_space = 512 * (__le64_to_cpu(sb->data_offset) -
					__le64_to_cpu(sb->super_offset));
		/* leave another 4k for superblock */
		total_bm_space = total_bm_space - 4096;

		if (bm_space_per_node * st->nodes > total_bm_space) {
			pr_err("Warning: The max num of nodes can't exceed %llu\n",
				total_bm_space / bm_space_per_node);
			return -ENOMEM;
		}

		bms->nodes = __cpu_to_le32(st->nodes);
		break;
	case NoUpdate:
	default:
		break;
	}

	init_afd(&afd, fd);

	locate_bitmap1(st, fd, 0);

	if (posix_memalign(&buf, 4096, 4096))
		return -ENOMEM;

	do {
		/* Only the bitmap[0] should resync
		 * whole device on initial assembly
		 */
		if (i)
			memset(buf, 0x00, 4096);
		else
			memset(buf, 0xff, 4096);
		memcpy(buf, (char *)bms, sizeof(bitmap_super_t));

		/*
		 * use 4096 boundary if bitmap_offset is aligned
		 * with 8 sectors, then it should compatible with
		 * older mdadm.
		 */
		if (__le32_to_cpu(sb->bitmap_offset) & 7)
			towrite = calc_bitmap_size(bms, 512);
		else
			towrite = calc_bitmap_size(bms, 4096);
		while (towrite > 0) {
			n = towrite;
			if (n > 4096)
				n = 4096;
			n = awrite(&afd, buf, n);
			if (n > 0)
				towrite -= n;
			else
				break;
			if (i)
				memset(buf, 0x00, 4096);
			else
				memset(buf, 0xff, 4096);
		}
		fsync(fd);
		if (towrite) {
			rv = -2;
			break;
		}
	} while (++i < __le32_to_cpu(bms->nodes));

	free(buf);
	return rv;
}

static void free_super1(struct supertype *st)
{

	if (st->sb)
		free(st->sb);
	while (st->info) {
		struct devinfo *di = st->info;
		st->info = di->next;
		if (di->fd >= 0)
			close(di->fd);
		free(di);
	}
	st->sb = NULL;
}

static int validate_geometry1(struct supertype *st, int level,
			      int layout, int raiddisks,
			      int *chunk, unsigned long long size,
			      unsigned long long data_offset,
			      char *subdev, unsigned long long *freesize,
			      int consistency_policy, int verbose)
{
	unsigned long long ldsize, devsize;
	int bmspace;
	unsigned long long headroom;
	int fd;

	if (level == LEVEL_CONTAINER) {
		if (verbose)
			pr_err("1.x metadata does not support containers\n");
		return 0;
	}
	if (*chunk == UnSet)
		*chunk = DEFAULT_CHUNK;

	if (!subdev)
		return 1;

	if (st->minor_version < 0)
		/* not specified, so time to set default */
		st->minor_version = 2;

	fd = open(subdev, O_RDONLY|O_EXCL, 0);
	if (fd < 0) {
		if (verbose)
			pr_err("super1.x cannot open %s: %s\n",
				subdev, strerror(errno));
		return 0;
	}

	if (!get_dev_size(fd, subdev, &ldsize)) {
		close(fd);
		return 0;
	}
	close(fd);

	devsize = ldsize >> 9;
	if (devsize < 24) {
		*freesize = 0;
		return 0;
	}

	/* creating:  allow suitable space for bitmap or PPL */
	if (consistency_policy == CONSISTENCY_POLICY_PPL)
		bmspace = MULTIPLE_PPL_AREA_SIZE_SUPER1 >> 9;
	else
		bmspace = choose_bm_space(devsize);

	if (data_offset == INVALID_SECTORS)
		data_offset = st->data_offset;
	if (data_offset == INVALID_SECTORS)
		switch (st->minor_version) {
		case 0:
			data_offset = 0;
			break;
		case 1:
		case 2:
			/* Choose data offset appropriate for this device
			 * and use as default for whole array.
			 * The data_offset must allow for bitmap space
			 * and base metadata, should allow for some headroom
			 * for reshape, and should be rounded to multiple
			 * of 1M.
			 * Headroom is limited to 128M, but aim for about 0.1%
			 */
			headroom = 128*1024*2;
			while ((headroom << 10) > devsize &&
			       (*chunk == 0 ||
				headroom / 2 >= ((unsigned)(*chunk)*2)*2))
				headroom >>= 1;
			data_offset = 12*2 + bmspace + headroom;
			#define ONE_MEG (2*1024)
			data_offset = ROUND_UP(data_offset, ONE_MEG);
			break;
		}
	if (st->data_offset == INVALID_SECTORS)
		st->data_offset = data_offset;
	switch(st->minor_version) {
	case 0: /* metadata at end.  Round down and subtract space to reserve */
		devsize = (devsize & ~(4ULL*2-1));
		/* space for metadata, bblog, bitmap/ppl */
		devsize -= 8*2 + 8 + bmspace;
		break;
	case 1:
	case 2:
		devsize -= data_offset;
		break;
	}
	*freesize = devsize;
	return 1;
}

void *super1_make_v0(struct supertype *st, struct mdinfo *info, mdp_super_t *sb0)
{
	/* Create a v1.0 superblock based on 'info'*/
	void *ret;
	struct mdp_superblock_1 *sb;
	int i;
	unsigned long long offset;

	if (posix_memalign(&ret, 4096, 1024) != 0)
		return NULL;
	sb = ret;
	memset(ret, 0, 1024);
	sb->magic = __cpu_to_le32(MD_SB_MAGIC);
	sb->major_version = __cpu_to_le32(1);

	copy_uuid(sb->set_uuid, info->uuid, super1.swapuuid);
	sprintf(sb->set_name, "%d", sb0->md_minor);
	sb->ctime = __cpu_to_le32(info->array.ctime+1);
	sb->level = __cpu_to_le32(info->array.level);
	sb->layout = __cpu_to_le32(info->array.layout);
	sb->size = __cpu_to_le64(info->component_size);
	sb->chunksize = __cpu_to_le32(info->array.chunk_size/512);
	sb->raid_disks = __cpu_to_le32(info->array.raid_disks);
	if (info->array.level > 0)
		sb->data_size = sb->size;
	else
		sb->data_size = st->ss->avail_size(st, st->devsize/512, 0);
	sb->resync_offset = MaxSector;
	sb->max_dev = __cpu_to_le32(MD_SB_DISKS);
	sb->dev_number = __cpu_to_le32(info->disk.number);
	sb->utime = __cpu_to_le64(info->array.utime);

	offset = st->devsize/512 - 8*2;
	offset &= ~(4*2-1);
	sb->super_offset = __cpu_to_le64(offset);
	//*(__u64*)(st->other + 128 + 8 + 8) = __cpu_to_le64(offset);

	random_uuid(sb->device_uuid);

	for (i = 0; i < MD_SB_DISKS; i++) {
		int state = sb0->disks[i].state;
		sb->dev_roles[i] = MD_DISK_ROLE_SPARE;
		if ((state & (1<<MD_DISK_SYNC)) &&
		    !(state & (1<<MD_DISK_FAULTY)))
			sb->dev_roles[i] = __cpu_to_le16(sb0->disks[i].raid_disk);
	}
	sb->sb_csum = calc_sb_1_csum(sb);
	return ret;
}

struct superswitch super1 = {
	.examine_super = examine_super1,
	.brief_examine_super = brief_examine_super1,
	.export_examine_super = export_examine_super1,
	.detail_super = detail_super1,
	.brief_detail_super = brief_detail_super1,
	.export_detail_super = export_detail_super1,
	.write_init_super = write_init_super1,
	.validate_geometry = validate_geometry1,
	.add_to_super = add_to_super1,
	.examine_badblocks = examine_badblocks_super1,
	.copy_metadata = copy_metadata1,
	.write_init_ppl = write_init_ppl1,
	.match_home = match_home1,
	.uuid_from_super = uuid_from_super1,
	.getinfo_super = getinfo_super1,
	.container_content = container_content1,
	.update_super = update_super1,
	.init_super = init_super1,
	.store_super = store_super1,
	.compare_super = compare_super1,
	.load_super = load_super1,
	.match_metadata_desc = match_metadata_desc1,
	.avail_size = avail_size1,
	.add_internal_bitmap = add_internal_bitmap1,
	.locate_bitmap = locate_bitmap1,
	.write_bitmap = write_bitmap1,
	.free_super = free_super1,
#if __BYTE_ORDER == BIG_ENDIAN
	.swapuuid = 0,
#else
	.swapuuid = 1,
#endif
	.name = "1.x",
};
