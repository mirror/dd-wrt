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
#include <inttypes.h>
#include "global.h"

struct mdp_superblock_1 {
	/* constant array information - 128 bytes */
	unsigned int magic; /* MD_SB_MAGIC: 0xa92b4efc - little endian */
	unsigned int major_version; /* 1 */
	unsigned int feature_map; /* bit 0 set if 'bitmap_offset' is meaningful */
	unsigned int pad0; /* always set to 0 when writing */

	unsigned char set_uuid[16]; /* user-space generated. */
	char set_name[32]; /* set and interpreted by user-space */

	unsigned long long ctime; /* lo 40 bits are seconds, top 24 are microseconds or 0 */
	unsigned int level; /* -4 (multipath), -1 (linear), 0,1,4,5 */
	unsigned int layout; /* only for raid5 and raid10 currently */
	unsigned long long size; /* used size of component devices, in 512byte sectors */

	unsigned int chunksize; /* in 512byte sectors */
	unsigned int raid_disks;
};

int detect_linux_md(SECTION *section, int level)
{
	unsigned char *buf;
	char s[256];
	if (get_buffer(section, 4096, 1024, (void **)&buf) < 1024)
		return 0;
	struct mdp_superblock_1 *super = (struct mdp_superblock_1 *)buf;
	if (get_le_long(&super->magic) == 0xa92b4efc) {
		print_line(level, "Linux MD Raid Version %d Level %d Disks %d", get_le_long(&super->major_version),
			   get_le_long(&super->level), get_le_long(&super->raid_disks));
		if (super->set_name[0])
			print_line(level + 1, "Volume name \"%s\"", super->set_name);
		format_uuid(super->set_uuid, s);
		print_line(level + 1, "UUID %s", s);
		format_blocky_size(s, get_le_quad(&super->size), 512, "blocks", NULL);
		print_line(level + 1, "Component size %s", s);
		return 1;
	}
	return 0;
}

/*
 * ext2/ext3/ext4 file system
 */
int detect_ext234(SECTION *section, int level)
{
	unsigned char *buf;
	char s[256];
	int fslevel, is_journal, is_dev;
	u4 blocksize;
	u8 blockcount;

	if (get_buffer(section, 1024, 1024, (void **)&buf) < 1024)
		return 0;

	if (get_le_short(buf + 56) == 0xEF53) {
		fslevel = 2;
		is_journal = 0;
		is_dev = 0;
		/* Ext3/4 external journal: INCOMPAT feature JOURNAL_DEV */
		if (get_le_long(buf + 96) & 0x0008) {
			is_journal = 1;
			fslevel = 3; /* at least ext3, ext2 has no journalling */
		}
		/* Ext3/4 COMPAT feature: HAS_JOURNAL */
		if (get_le_long(buf + 92) & 0x0004)
			fslevel = 3;
		/* Ext4 INCOMPAT features: EXTENTS, 64BIT, FLEX_BG */
		if (get_le_long(buf + 96) & 0x02C0)
			fslevel = 4;
		/* Ext4 RO_COMPAT features: HUGE_FILE, GDT_CSUM, DIR_NLINK, EXTRA_ISIZE */
		if (get_le_long(buf + 100) & 0x0078)
			fslevel = 4;
		/* Ext4 sets min_extra_isize even on external journals */
		if (get_le_short(buf + 348) >= 0x1c)
			fslevel = 4;
		/* Ext4dev TEST_FILESYS flag */
		if (get_le_long(buf + 352) & 0x0004)
			is_dev = 1;

		print_line(level, "Ext%d%s %s", fslevel, is_dev ? "dev" : "", is_journal ? "external journal" : "file system");

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
		return fslevel;
	}
	return 0;
}

/*
 * btrfs file system
 */

int detect_btrfs(SECTION *section, int level)
{
	unsigned char *buf;
	char s[258];

	if (get_buffer(section, 64 * 1024, 1024, (void **)&buf) < 1024)
		return 0;

	if (memcmp(buf + 64, "_BHRfS_M", 8) == 0) {
		print_line(level, "Btrfs file system");

		get_string(buf + 299, 256, s);
		if (s[0])
			print_line(level + 1, "Volume name \"%s\"", s);

		format_uuid(buf + 32, s);
		print_line(level + 1, "UUID %s", s);

		format_size(s, get_le_quad(buf + 0x70));
		print_line(level + 1, "Volume size %s", s);
		return 1;
	}
	return 0;
}

#define VDEV_LABEL_UBERBLOCK (128 * 1024ULL)
#define VDEV_LABEL_NVPAIR (16 * 1024ULL)
#define VDEV_LABEL_SIZE (256 * 1024ULL)
#define UBERBLOCK_MAGIC 0x00bab10c /* oo-ba-bloc!  */
#define UBERBLOCK_SIZE 1024ULL
#define UBERBLOCKS_COUNT 128
#define ZFS_TRIES 64
#define ZFS_WANT 4
struct zfs_uberblock {
	uint64_t ub_magic; /* UBERBLOCK_MAGIC              */
	uint64_t ub_version; /* SPA_VERSION                  */
	uint64_t ub_txg; /* txg of last sync             */
	uint64_t ub_guid_sum; /* sum of all vdev guids        */
	uint64_t ub_timestamp; /* UTC time of last sync        */
	char ub_rootbp; /* MOS objset_phys_t            */
} __attribute__((packed));

#define DATA_TYPE_UINT64 8
#define DATA_TYPE_STRING 9

struct nvpair {
	uint32_t nvp_size;
	uint32_t nvp_unkown;
	uint32_t nvp_namelen;
	char nvp_name[0]; /* aligned to 4 bytes */
	/* aligned ptr array for string arrays */
	/* aligned array of data for value */
};

struct nvstring {
	uint32_t nvs_type;
	uint32_t nvs_elem;
	uint32_t nvs_strlen;
	unsigned char nvs_string[0];
};

struct nvuint64 {
	uint32_t nvu_type;
	uint32_t nvu_elem;
	uint64_t nvu_value;
};

struct nvlist {
	uint32_t nvl_unknown[3];
	struct nvpair nvl_nvpair;
};

static int zfs_process_value(int level, char *name, size_t namelen, void *value, size_t max_value_size)
{
	if (strncmp(name, "name", namelen) == 0 && sizeof(struct nvstring) <= max_value_size) {
		struct nvstring *nvs = value;
		uint32_t nvs_type = be32_to_cpu(nvs->nvs_type);
		uint32_t nvs_strlen = be32_to_cpu(nvs->nvs_strlen);

		if (nvs_type != DATA_TYPE_STRING || (uint64_t)nvs_strlen + sizeof(*nvs) > max_value_size)
			return 0;

		print_line(level + 1, "ZFS Label: %s", nvs->nvs_string);

		return 1;
	} else if (strncmp(name, "guid", namelen) == 0 && sizeof(struct nvuint64) <= max_value_size) {
		struct nvuint64 *nvu = value;
		uint32_t nvu_type = be32_to_cpu(nvu->nvu_type);
		uint64_t nvu_value;

		memcpy(&nvu_value, &nvu->nvu_value, sizeof(nvu_value));
		nvu_value = be64_to_cpu(nvu_value);

		if (nvu_type != DATA_TYPE_UINT64)
			return 0;

		print_line(level + 1, "ZFS UUID_SUB: %" PRIu64, nvu_value);

		return 1;
	} else if (strncmp(name, "pool_guid", namelen) == 0 && sizeof(struct nvuint64) <= max_value_size) {
		struct nvuint64 *nvu = value;
		uint32_t nvu_type = be32_to_cpu(nvu->nvu_type);
		uint64_t nvu_value;

		memcpy(&nvu_value, &nvu->nvu_value, sizeof(nvu_value));
		nvu_value = be64_to_cpu(nvu_value);

		if (nvu_type != DATA_TYPE_UINT64)
			return 0;

		char s[256];
		format_guid(&nvu_value, s);
		print_line(level + 1, "ZFS POOL_GUID %s", s);

		return 1;
	}

	return 0;
}

static void zfs_extract_guid_name(SECTION *section, int level, loff_t offset)
{
	unsigned char *p;
	struct nvlist *nvl;
	struct nvpair *nvp;
	size_t left = 4096;
	int found = 0;

	offset = (offset & ~(VDEV_LABEL_SIZE - 1)) + VDEV_LABEL_NVPAIR;

	/* Note that we currently assume that the desired fields are within
	 * the first 4k (left) of the nvlist.  This is true for all pools
	 * I've seen, and simplifies this code somewhat, because we don't
	 * have to handle an nvpair crossing a buffer boundary. */
	if (get_buffer(section, offset, left, (void **)&p) < left)
		return;

	nvl = (struct nvlist *)p;
	nvp = &nvl->nvl_nvpair;
	left -= (unsigned char *)nvp - p; /* Already used up 12 bytes */

	while (left > sizeof(*nvp) && nvp->nvp_size != 0 && found < 3) {
		uint32_t nvp_size = be32_to_cpu(nvp->nvp_size);
		uint32_t nvp_namelen = be32_to_cpu(nvp->nvp_namelen);
		uint64_t namesize = ((uint64_t)nvp_namelen + 3) & ~3;
		size_t max_value_size;
		void *value;

		/* nvpair fits in buffer and name fits in nvpair? */
		if (nvp_size > left || namesize + sizeof(*nvp) > nvp_size)
			break;

		max_value_size = nvp_size - (namesize + sizeof(*nvp));
		value = nvp->nvp_name + namesize;

		found += zfs_process_value(level, nvp->nvp_name, nvp_namelen, value, max_value_size);

		left -= nvp_size;

		nvp = (struct nvpair *)((char *)nvp + nvp_size);
	}
}

static int find_uberblocks(const void *label, loff_t *ub_offset, int *swap_endian)
{
	uint64_t swab_magic = bswap_64((uint64_t)UBERBLOCK_MAGIC);
	struct zfs_uberblock *ub;
	int i, found = 0;
	loff_t offset = VDEV_LABEL_UBERBLOCK;

	for (i = 0; i < UBERBLOCKS_COUNT; i++, offset += UBERBLOCK_SIZE) {
		ub = (struct zfs_uberblock *)((char *)label + offset);

		if (ub->ub_magic == UBERBLOCK_MAGIC) {
			*ub_offset = offset;
			*swap_endian = 0;
			found++;
		}

		if (ub->ub_magic == swab_magic) {
			*ub_offset = offset;
			*swap_endian = 1;
			found++;
		}
	}

	return found;
}

int detect_zfs(SECTION *section, int level)
{
	int swab_endian = 0;
	struct zfs_uberblock *ub;
	loff_t offset, ub_offset = 0;
	int label_no, found = 0, found_in_label;
	void *label;
	loff_t blk_align = (64 * 1024 * 1024 % (256 * 1024ULL));

	/* Look for at least 4 uberblocks to ensure a positive match */
	for (label_no = 0; label_no < 4; label_no++) {
		switch (label_no) {
		case 0: // jump to L0
			offset = 0;
			break;
		case 1: // jump to L1
			offset = VDEV_LABEL_SIZE;
			break;
		case 2: // jump to L2
			offset = 64 * 1024 * 1024 - 2 * VDEV_LABEL_SIZE - blk_align;
			break;
		case 3: // jump to L3
			offset = 64 * 1024 * 1024 - VDEV_LABEL_SIZE - blk_align;
			break;
		}

		if (get_buffer(section, offset, VDEV_LABEL_SIZE, (void **)&label) < VDEV_LABEL_SIZE)
			return 0;

		found_in_label = find_uberblocks(label, &ub_offset, &swab_endian);

		if (found_in_label > 0) {
			found += found_in_label;
			ub = (struct zfs_uberblock *)((char *)label + ub_offset);
			ub_offset += offset;

			if (found >= ZFS_WANT)
				break;
		}
	}

	if (found < ZFS_WANT)
		return 0;
	print_line(level, "ZFS file system v%" PRIu64 " Endian: %s", swab_endian ? bswap_64(ub->ub_version) : ub->ub_version,
		   swab_endian ? "big" : "little");

	zfs_extract_guid_name(section, level, offset);
	return 1;
}

int detect_f2fs(SECTION *section, int level)
{
	unsigned char *buf;
	char s[258];

	if (get_buffer(section, 0x400, 1024, (void **)&buf) < 1024)
		return 0;

	if (get_le_long(buf) == 0xF2F52010) {
		print_line(level, "F2FS file system v%d.%d", get_le_short(buf + 4), get_le_short(buf + 6));

		format_utf16_le(buf + 108 + 16, 512, s);

		if (s[0])
			print_line(level + 1, "Volume name \"%s\"", s);

		format_uuid(buf + 108, s);
		print_line(level + 1, "E2FS %s", s);
		return 1;
	}
	return 0;
}

/*
 * ReiserFS file system
 */

int detect_reiser(SECTION *section, int level)
{
	unsigned char *buf;
	int i, at, newformat;
	int offsets[3] = { 8, 64, -1 };
	char s[256];
	u8 blockcount;
	u4 blocksize;
	int found = 0;
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
				print_line(level, "ReiserFS file system (old 3.5 format, non-standard journal, starts at %d KiB)",
					   at);
			} else if (newformat == 2) {
				print_line(level, "ReiserFS file system (new 3.6 format, non-standard journal, starts at %d KiB)",
					   at);
				newformat = 1;
			} else {
				print_line(level, "ReiserFS file system (v3 magic, but unknown version %d, starts at %d KiB)",
					   newformat, at);
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
		found = 1;
		/* TODO: print hash code */
	}
	return found;
}

/*
 * Reiser4 file system
 */

int detect_reiser4(SECTION *section, int level)
{
	unsigned char *buf;
	char s[256];
	int layout_id;
	char layout_name[64];
	u4 blocksize;
	u8 blockcount;

	if (get_buffer(section, 16 * 4096, 1024, (void **)&buf) < 1024)
		return 0;

	/* check signature */
	if (memcmp(buf, "ReIsEr4", 7) != 0)
		return 0;

	/* get data from master superblock */
	layout_id = get_le_short(buf + 16);
	blocksize = get_le_short(buf + 18);
	if (layout_id == 0)
		strcpy(layout_name, "4.0 layout");
	else
		sprintf(layout_name, "Unknown layout with ID %d", layout_id);

	format_size(s, blocksize);
	print_line(level, "Reiser4 file system (%s, block size %s)", layout_name, s);

	/* get label and UUID */
	get_string(buf + 36, 16, s);
	if (s[0])
		print_line(level + 1, "Volume name \"%s\"", s);

	format_uuid(buf + 20, s);
	print_line(level + 1, "UUID %s", s);

	if (layout_id == 0) {
		/* read 4.0 superblock */
		if (get_buffer(section, 17 * 4096, 1024, (void **)&buf) < 1024)
			return 0;
		if (memcmp(buf + 52, "ReIsEr40FoRmAt", 14) != 0) {
			print_line(level + 1, "Superblock for 4.0 format missing");
			return 0;
		}

		blockcount = get_le_quad(buf);
		format_blocky_size(s, blockcount, blocksize, "blocks", NULL);
		print_line(level + 1, "Volume size %s", s);
	}
	return 1;
}

/*
 * Linux RAID persistent superblock
 */

static char *levels[] = { "Multipath", "\'HSM\'", "\'translucent\'", "Linear", "RAID0", "RAID1", NULL, NULL, "RAID4(?)", "RAID5" };

int detect_linux_raid(SECTION *section, int level)
{
	unsigned char *buf;
	unsigned long long pos;
	int rlevel, nr_disks, raid_disks, spare;
	u1 uuid[16];
	char s[256];

	/* don't do this if:
	 *  - the size is unknown (0)
	 *  - the size is too small for the calculation
	 *  - it is inefficient to read from the end of the source
	 */
	/* get RAID superblock from the end of the device */
	if (section->size < 65536 || section->source->sequential)
		return 0;
	/* get RAID superblock from the end of the device */
	pos = (section->size & ~65535) - 65536;
	if (get_buffer(section, pos, 4096, (void **)&buf) < 4096)
		return 0;

	/* signature */
	if (get_le_long(buf) != 0xa92b4efc)
		return 0;

	print_line(level, "Linux RAID disk, version %lu.%lu.%lu", get_le_long(buf + 4), get_le_long(buf + 8),
		   get_le_long(buf + 12));

	/* get some data */
	rlevel = (int)(long)get_le_long(buf + 28); /* is signed, actually */
	nr_disks = get_le_long(buf + 36);
	raid_disks = get_le_long(buf + 40);
	spare = nr_disks - raid_disks;

	/* find the name for the personality in the table */
	if (rlevel < -4 || rlevel > 5 || levels[rlevel + 4] == NULL) {
		print_line(level + 1, "Unknown RAID level %d using %d regular %d spare disks", rlevel, raid_disks, spare);
	} else {
		print_line(level + 1, "%s set using %d regular %d spare disks", levels[rlevel + 4], raid_disks, spare);
	}

	/* get the UUID */
	memcpy(uuid, buf + 5 * 4, 4);
	memcpy(uuid + 4, buf + 13 * 4, 3 * 4);
	format_uuid(uuid, s);
	print_line(level + 1, "RAID set UUID %s", s);
	return 1;
}

/*
 * Linux LVM1
 */

int detect_linux_lvm(SECTION *section, int level)
{
	unsigned char *buf;
	char s[256];
	int minor_version, pv_number;
	u8 pe_size, pe_count, pe_start;

	if (get_buffer(section, 0, 1024, (void **)&buf) < 1024)
		return 0;

	/* signature */
	if (buf[0] != 'H' || buf[1] != 'M')
		return 0;
	/* helpful sanity check... */
	if (get_le_long(buf + 36) == 0 || get_le_long(buf + 40) == 0)
		return 0;

	minor_version = get_le_short(buf + 2);
	print_line(level, "Linux LVM1 volume, version %d%s", minor_version,
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
	if (level >= 0 && pe_start > 0) {
		analyze_recursive(section, level + 1, pe_start, 0, 0);
		/* TODO: elaborate on this by reading the PE allocation map */
	}
	return 1;
}

/*
 * Linux LVM2
 */

int detect_linux_lvm2(SECTION *section, int level)
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
			print_line(level, "LABELONE label at sector %d, unknown type \"%s\"", at, s);
			return 0;
		}

		print_line(level, "Linux LVM2 volume, version 001");
		print_line(level + 1, "LABELONE label at sector %d", at);

		if (labeloffset >= 512 || labelsector > 256 || labelsector != at) {
			print_line(level + 1, "LABELONE data inconsistent, aborting analysis");
			return 0;
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
			return 0;

		if (get_buffer(section, mdoffset, mdsize, (void **)&buf) < mdsize)
			return 0;

		if (memcmp(buf + 4, " LVM2 x[5A%r0N*>", 16) != 0)
			return 0;
		mda_version = get_le_long(buf + 20);

		print_line(level + 1, "Meta-data version %d", mda_version);

		/* TODO: parse the metadata area (big task...) */

		return 1;
	}
	return 0;
}

/*
 * Linux swap area
 */

int detect_linux_swap(SECTION *section, int level)
{
	int i, en, pagesize;
	unsigned char *buf;
	u4 version, pages;
	char s[256];
	int pagesizes[] = { 4096, 8192, 0 };
	int found = 0;
	for (i = 0; pagesizes[i]; i++) {
		pagesize = pagesizes[i];

		if (get_buffer(section, pagesize - 512, 512, (void **)&buf) != 512)
			break; /* assumes page sizes increase through the loop */

		if (memcmp((char *)buf + 512 - 10, "SWAP-SPACE", 10) == 0) {
			print_line(level, "Linux swap, version 1, %d KiB pages", pagesize >> 10);
			found = 1;
		}
		if (memcmp((char *)buf + 512 - 10, "SWAPSPACE2", 10) == 0) {
			if (get_buffer(section, 1024, 512, (void **)&buf) != 512)
				break; /* really shouldn't happen */

			for (en = 0; en < 2; en++) {
				version = get_ve_long(en, buf);
				if (version >= 1 && version < 10)
					break;
			}
			if (en < 2) {
				print_line(level, "Linux swap, version 2, subversion %d, %d KiB pages, %s", (int)version,
					   pagesize >> 10, get_ve_name(en));
				if (version == 1) {
					pages = get_ve_long(en, buf + 4) - 1;
					format_blocky_size(s, pages, pagesize, "pages", NULL);
					print_line(level + 1, "Swap size %s", s);
				}
				found = 1;
			} else {
				print_line(level, "Linux swap, version 2, illegal subversion, %d KiB pages", pagesize >> 10);
				found = 1;
			}
		}
	}
	return found;
}

/*
 * various file systems
 */

int detect_linux_misc(SECTION *section, int level)
{
	int magic, fill, off, en;
	unsigned char *buf;
	char s[256];
	u8 size, blocks, blocksize;
	int found = 0;

	fill = get_buffer(section, 0, 2048, (void **)&buf);
	if (fill < 512)
		return 0;

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
			print_line(level, "Minix file system (v%d, %d chars)", version, namesize);
			if (version == 1)
				blocks = get_le_short(buf + 1024 + 2);
			else
				blocks = get_le_long(buf + 1024 + 20);
			blocks = (blocks - get_le_short(buf + 1024 + 8)) << get_le_short(buf + 1024 + 10);
			format_blocky_size(s, blocks, 1024, "blocks", NULL);
			print_line(level + 1, "Volume size %s", s);
			found = 1;
		}
	}

	/* Linux romfs */
	if (memcmp(buf, "-rom1fs-", 8) == 0) {
		size = get_be_long(buf + 8);
		print_line(level, "Linux romfs");
		print_line(level + 1, "Volume name \"%.300s\"", (char *)(buf + 16));
		format_size_verbose(s, size);
		print_line(level + 1, "Volume size %s", s);
		found = 1;
	}

	/* Linux cramfs */
	for (off = 0; off <= 512; off += 512) {
		if (fill < off + 512)
			break;
		for (en = 0; en < 2; en++) {
			if (get_ve_long(en, buf + off) == 0x28cd3d45) {
				print_line(level, "Linux cramfs, starts sector %d, %s", off >> 9, get_ve_name(en));

				get_string(buf + off + 48, 16, s);
				print_line(level + 1, "Volume name \"%s\"", s);

				size = get_ve_long(en, buf + off + 4);
				blocks = get_ve_long(en, buf + off + 40);
				format_size_verbose(s, size);
				print_line(level + 1, "Compressed size %s", s);
				format_blocky_size(s, blocks, 4096, "blocks", " -assumed-");
				print_line(level + 1, "Data size %s", s);
				found = 1;
			}
		}
	}

	/* Linux squashfs */
	for (en = 0; en < 2; en++) {
		if (get_ve_long(en, buf) == 0x73717368 || get_ve_long(en, buf) == 0x74717368) {
			int major, minor;

			major = get_ve_short(en, buf + 28);
			minor = get_ve_short(en, buf + 30);
			print_line(level, "Linux squashfs, version %d.%d, %s", major, minor, get_ve_name(en));

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
			found = 1;
		}
	}
	return found;
}

/*
 * various boot code signatures
 */

int detect_linux_loader(SECTION *section, int level)
{
	int fill, executable, id;
	unsigned char *buf;
	int found = 1;
	if (section->flags & FLAG_IN_DISKLABEL)
		return 0;

	fill = get_buffer(section, 0, 2048, (void **)&buf);
	if (fill < 512)
		return 0;

	executable = (get_le_short(buf + 510) == 0xaa55) ? 1 : 0;

	/* boot sector stuff */
	if (executable && (memcmp(buf + 2, "LILO", 4) == 0 || memcmp(buf + 6, "LILO", 4) == 0)) {
		print_line(level, "LILO boot loader");
		found = 1;
	}
	if (executable && memcmp(buf + 3, "SYSLINUX", 8) == 0) {
		print_line(level, "SYSLINUX boot loader");
		found = 1;
	}
	if (fill >= 1024 && find_memory(buf, fill, "ISOLINUX", 8) >= 0) {
		print_line(level, "ISOLINUX boot loader");
		found = 1;
	}

	/* we know GRUB a little better... */
	if (executable && find_memory(buf, 512, "Geom\0Hard Disk\0Read\0 Error", 26) >= 0) {
		if (buf[0x3e] == 3) {
			print_line(level, "GRUB boot loader, compat version %d.%d, boot drive 0x%02x", (int)buf[0x3e],
				   (int)buf[0x3f], (int)buf[0x40]);
			found = 1;
		} else if (executable && buf[0x1bc] == 2 && buf[0x1bd] <= 2) {
			id = buf[0x3e];
			if (id == 0x10) {
				print_line(level, "GRUB boot loader, compat version %d.%d, normal version", (int)buf[0x1bc],
					   (int)buf[0x1bd]);
				found = 1;
			} else if (id == 0x20) {
				print_line(level, "GRUB boot loader, compat version %d.%d, LBA version", (int)buf[0x1bc],
					   (int)buf[0x1bd]);
				found = 1;
			} else {
				print_line(level, "GRUB boot loader, compat version %d.%d", (int)buf[0x1bc], (int)buf[0x1bd]);
				found = 1;
			}
		} else {
			print_line(level, "GRUB boot loader, unknown compat version %d", buf[0x3e]);
			found = 1;
		}
	}

	/* Linux kernel loader */
	if (fill >= 1024 && memcmp(buf + 512 + 2, "HdrS", 4) == 0) {
		print_line(level, "Linux kernel build-in loader");
		found = 1;
	}

	/* Debian install floppy splitter */
	/* (not exactly boot code, but should be detected before gzip/tar) */
	if (memcmp(buf, "Floppy split ", 13) == 0) {
		char *name = (char *)buf + 32;
		char *number = (char *)buf + 164;
		char *total = (char *)buf + 172;
		print_line(level, "Debian floppy split, name \"%s\", disk %s of %s", name, number, total);
		found = 1;
	}
	return found;
}

/* EOF */
