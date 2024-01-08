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

int detect_apple_partmap(SECTION *section, int level)
{
	int i, sectorsize, magic, count;
	char s[256], append[64];
	unsigned char *buf;
	u8 start, size;

	/* partition maps only occur at the start of a device */
	/*  disabled to allow for APM maps in El Torito entries
	   if (section->pos != 0)
	   return 0;
	 */

	/*
	   if (buf[off] == 0x45 && buf[off+1] == 0x52) {
	   print_line(level, "HFS driver description map at sector %d", i);
	   }
	 */

	for (sectorsize = 512; sectorsize <= 4096; sectorsize <<= 1) {
		if (get_buffer(section, sectorsize, sectorsize, (void **)&buf) <
		    sectorsize)
			return 0;

		magic = get_be_short(buf);
		if (magic == 0x5453) {
			print_line(level, "Old-style Apple partition map");
			return 1;
		}
		if (magic != 0x504D)
			continue;

		/* get partition count and print info */
		count = get_be_long(buf + 4);
		print_line(level,
			   "Apple partition map, %d entries, %d byte sectors",
			   count, sectorsize);

		for (i = 1; i <= count; i++) {
			/* read the right sector */
			/* NOTE: the previous run through the loop might have called
			 *  get_buffer indirectly, invalidating our old pointer */
			if (i > 1 &&
			    get_buffer(section, i * sectorsize, sectorsize,
				       (void **)&buf) < sectorsize)
				return 0;

			/* check signature */
			if (get_be_short(buf) != 0x504D) {
				print_line(
					level,
					"Partition %d: invalid signature, skipping",
					i);
				continue;
			}

			/* get position and size */
			start = get_be_long(buf + 8);
			size = get_be_long(buf + 12);
			sprintf(append, " from %llu", start);
			format_blocky_size(s, size, sectorsize, "sectors",
					   append);
			print_line(level, "Partition %d: %s", i, s);

			/* get type */
			get_string(buf + 48, 32, s);
			print_line(level + 1, "Type \"%s\"", s);

			/* recurse for content detection */
			if (level >= 0 && start > count &&
			    size > 0) { /* avoid recursion on self */
				analyze_recursive(section, level + 1,
						  start * sectorsize,
						  size * sectorsize, 0);
			}
		}
		return 1; /* don't try bigger sector sizes */
	}
	return 0;
}

struct apfs_prange {
	unsigned long long pr_start_paddr;
	unsigned long long pr_block_count;
} __attribute__((packed));

struct apfs_obj_phys {
	/*00*/ unsigned long long o_cksum;
	/* Fletcher checksum */
	unsigned long long o_oid; /* Object-id */
	/*10*/ unsigned long long o_xid;
	/* Transaction ID */
	unsigned int o_type; /* Object type */
	unsigned int o_subtype; /* Object subtype */
} __attribute__((packed));

/* Indexes into a container superblock's array of counters */
enum {
	APFS_NX_CNTR_OBJ_CKSUM_SET = 0,
	APFS_NX_CNTR_OBJ_CKSUM_FAIL = 1,

	APFS_NX_NUM_COUNTERS = 32
};
#define APFS_NX_EPH_INFO_COUNT 4

#define APFS_NX_MAX_FILE_SYSTEMS 100

/*
 * On-disk representation of the container superblock
 */
struct apfs_nx_superblock {
	/*00*/ struct apfs_obj_phys nx_o;
	/*20*/ unsigned int nx_magic;
	unsigned int nx_block_size;
	unsigned long long nx_block_count;

	/*30*/ unsigned long long nx_features;
	unsigned long long nx_readonly_compatible_features;
	unsigned long long nx_incompatible_features;

	/*48*/ char nx_uuid[16];

	/*58*/ unsigned long long nx_next_oid;
	unsigned long long nx_next_xid;

	/*68*/ unsigned int nx_xp_desc_blocks;
	unsigned int nx_xp_data_blocks;
	/*70*/ unsigned long long nx_xp_desc_base;
	unsigned long long nx_xp_data_base;
	unsigned int nx_xp_desc_next;
	unsigned int nx_xp_data_next;
	/*88*/ unsigned int nx_xp_desc_index;
	unsigned int nx_xp_desc_len;
	unsigned int nx_xp_data_index;
	unsigned int nx_xp_data_len;

	/*98*/ unsigned long long nx_spaceman_oid;
	unsigned long long nx_omap_oid;
	unsigned long long nx_reaper_oid;

	/*B0*/ unsigned int nx_test_type;

	unsigned int nx_max_file_systems;
	/*B8*/ unsigned long long nx_fs_oid[APFS_NX_MAX_FILE_SYSTEMS];
	/*3D8*/ unsigned long long nx_counters[APFS_NX_NUM_COUNTERS];
	/*4D8*/ struct apfs_prange nx_blocked_out_prange;
	unsigned long long nx_evict_mapping_tree_oid;
	/*4F0*/ unsigned long long nx_flags;
	unsigned long long nx_efi_jumpstart;
	/*500*/ char nx_fusion_uuid[16];
	struct apfs_prange nx_keylocker;
	/*520*/ unsigned long long nx_ephemeral_info[APFS_NX_EPH_INFO_COUNT];

	/*540*/ unsigned long long nx_test_oid;

	unsigned long long nx_fusion_mt_oid;
	/*550*/ unsigned long long nx_fusion_wbc_oid;
	struct apfs_prange nx_fusion_wbc;
} __attribute__((packed));

struct apfs_wrapped_meta_crypto_state {
	unsigned short major_version;
	unsigned short minor_version;
	unsigned int cpflags;
	unsigned int persistent_class;
	unsigned int key_os_version;
	unsigned short key_revision;
	unsigned short unused;
} __attribute__((packed));

#define APFS_MODIFIED_NAMELEN 32

/*
 * Structure containing information about a program that modified the volume
 */
struct apfs_modified_by {
	char id[APFS_MODIFIED_NAMELEN];
	unsigned long long timestamp;
	unsigned long long last_xid;
} __attribute__((packed));

#define APFS_MAX_HIST 8
#define APFS_VOLNAME_LEN 256

struct apfs_superblock {
	/*00*/ struct apfs_obj_phys apfs_o;

	/*20*/ unsigned int apfs_magic;
	unsigned int apfs_fs_index;

	/*28*/ unsigned long long apfs_features;
	unsigned long long apfs_readonly_compatible_features;
	unsigned long long apfs_incompatible_features;

	/*40*/ unsigned long long apfs_unmount_time;

	unsigned long long apfs_fs_reserve_block_count;
	unsigned long long apfs_fs_quota_block_count;
	unsigned long long apfs_fs_alloc_count;

	/*60*/ struct apfs_wrapped_meta_crypto_state apfs_meta_crypto;

	/*74*/ unsigned int apfs_root_tree_type;
	unsigned int apfs_extentref_tree_type;
	unsigned int apfs_snap_meta_tree_type;

	/*80*/ unsigned long long apfs_omap_oid;
	unsigned long long apfs_root_tree_oid;
	unsigned long long apfs_extentref_tree_oid;
	unsigned long long apfs_snap_meta_tree_oid;

	/*A0*/ unsigned long long apfs_revert_to_xid;
	unsigned long long apfs_revert_to_sblock_oid;

	/*B0*/ unsigned long long apfs_next_obj_id;

	/*B8*/ unsigned long long apfs_num_files;
	unsigned long long apfs_num_directories;
	unsigned long long apfs_num_symlinks;
	unsigned long long apfs_num_other_fsobjects;
	unsigned long long apfs_num_snapshots;

	/*E0*/ unsigned long long apfs_total_blocks_alloced;
	unsigned long long apfs_total_blocks_freed;

	/*F0*/ char apfs_vol_uuid[16];
	/*100*/ unsigned long long apfs_last_mod_time;

	unsigned long long apfs_fs_flags;

	/*110*/ struct apfs_modified_by apfs_formatted_by;
	/*140*/ struct apfs_modified_by apfs_modified_by[APFS_MAX_HIST];

	/*2C0*/ char apfs_volname[APFS_VOLNAME_LEN];
	/*3C0*/ unsigned int apfs_next_doc_id;

	unsigned short apfs_role;
	unsigned short reserved;

	/*3C8*/ unsigned long long apfs_root_to_xid;
	unsigned long long apfs_er_state_oid;
} __attribute__((packed));

#define FIRST_VOL_BNO 20002
#define APPLE_NX_DEFAULT_BLOCKSIZE 4096
#define APFS_NX_MAGIC 0x4253584E

int detect_apfs_volume(SECTION *section, int level)
{
	char s[256], t[1026];
	struct apfs_superblock *sb;
	struct apfs_nx_superblock *nxsb;
	if (get_buffer(section, 0, sizeof(*nxsb), (void **)&nxsb) <
	    sizeof(*nxsb))
		return 0;
	if (le32toh(nxsb->nx_magic) == APFS_NX_MAGIC) {
		print_line(level, "APFS Container");
		format_blocky_size(s, le64toh(nxsb->nx_block_count),
				   le32toh(nxsb->nx_block_size), "blocks",
				   NULL);
		print_line(level + 1, "Size %s", s);
		format_uuid(nxsb->nx_uuid, s);
		print_line(level + 1, "UUID %s", s);
		if (get_buffer(section,
			       FIRST_VOL_BNO * le32toh(nxsb->nx_block_size),
			       sizeof(*sb), (void **)&sb) < sizeof(*sb))
			return 0;
		if (le32toh(sb->apfs_magic) == 0x42535041) {
			print_line(level, "APFS Volume");
			strcpy(s, sb->apfs_volname);
			format_ascii(s, t);
			print_line(level + 1, "Volume name \"%s\"", t);

			format_uuid(sb->apfs_vol_uuid, s);
			print_line(level + 1, "UUID %s", s);
		}
		return 1;
	}
	return 0;
}

/*
 * Apple volume formats: MFS, HFS, HFS Plus
 */

int detect_apple_volume(SECTION *section, int level)
{
	char s[256], t[1026];
	unsigned char *buf;
	u2 magic, volnamelen;
	u4 blocksize, blockstart;
	u8 blockcount, offset;
	u8 catalogstart, cataloglength;
	u4 firstleafnode, nodesize;

	if (get_buffer(section, 1024, 512, (void **)&buf) < 512)
		return 0;

	magic = get_be_short(buf);
	//version = get_be_short(buf + 2);

	if (magic == 0xD2D7) {
		print_line(level, "MFS file system");
		return 1;
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

		if (level >= 0 && get_be_short(buf + 0x7c) == 0x482B) {
			print_line(level, "HFS wrapper for HFS Plus");

			offset = (u8)get_be_short(buf + 0x7e) * blocksize +
				 (u8)blockstart * 512;
			/* TODO: size */

			if (level >= 0)
				analyze_recursive(section, level + 1, offset, 0,
						  0);
		}
		return 1;

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
		if (get_buffer(section, catalogstart, 4096, (void **)&buf) <
		    4096)
			return 0;
		firstleafnode = get_be_long(buf + 24);
		nodesize = get_be_short(buf + 32);
		if (nodesize < 4096)
			return 0; /* illegal value */

		/* read first lead node */
		if ((firstleafnode + 1) * nodesize > cataloglength)
			return 0; /* the location is beyond the end of the catalog */
		if (get_buffer(section, catalogstart + firstleafnode * nodesize,
			       nodesize, (void **)&buf) < nodesize)
			return 0;

		/* the first record in this leaf node should be for parent id 1 */
		if (buf[8] != 0xff)
			return 0; /* not a leaf node */
		if (get_be_short(buf + 14) <= 6)
			return 0; /* key of first record is too short to contain a name */
		if (get_be_long(buf + 16) != 1)
			return 0; /* parent folder id is not "root parent" */
		volnamelen = get_be_short(buf + 20);
		format_utf16_be(buf + 22, volnamelen * 2, t);
		print_line(level + 1, "Volume name \"%s\"", t);
		return 1;
	}
	return 0;
}

/*
 * Apple UDIF disk images
 */

int detect_udif(SECTION *section, int level)
{
	u8 pos;
	unsigned char *buf;

	if (section->size < 1024 || section->source->sequential)
		return 0;

	pos = section->size - 512;
	if (get_buffer(section, pos, 512, (void **)&buf) < 512)
		return 0;

	if (memcmp(buf, "koly", 4) == 0) {
		print_line(
			level,
			"Apple UDIF disk image, content detection may or may not work...");
		return 1;
	}
	return 0;
}

/* EOF */
