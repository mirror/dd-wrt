/*
   md_p.h : physical layout of Linux RAID devices
          Copyright (C) 1996-98 Ingo Molnar, Gadi Oxman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   You should have received a copy of the GNU General Public License
   (for example /usr/src/linux/COPYING); if not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _MD_P_H
#define _MD_P_H

/*
 * RAID superblock.
 *
 * The RAID superblock maintains some statistics on each RAID configuration.
 * Each real device in the RAID set contains it near the end of the device.
 * Some of the ideas are copied from the ext2fs implementation.
 *
 * We currently use 4096 bytes as follows:
 *
 *	word offset	function
 *
 *	   0  -    31	Constant generic RAID device information.
 *        32  -    63   Generic state information.
 *	  64  -   127	Personality specific information.
 *	 128  -   511	12 32-words descriptors of the disks in the raid set.
 *	 512  -   911	Reserved.
 *	 912  -  1023	Disk specific descriptor.
 */

/*
 * If x is the real device size in bytes, we return an apparent size of:
 *
 *	y = (x & ~(MD_RESERVED_BYTES - 1)) - MD_RESERVED_BYTES
 *
 * and place the 4kB superblock at offset y.
 */
#define MD_RESERVED_BYTES		(64 * 1024)
#define MD_RESERVED_SECTORS		(MD_RESERVED_BYTES / 512)
#define MD_RESERVED_BLOCKS		(MD_RESERVED_BYTES / BLOCK_SIZE)

#define MD_NEW_SIZE_SECTORS(x)		((x & ~(MD_RESERVED_SECTORS - 1)) - MD_RESERVED_SECTORS)
#define MD_NEW_SIZE_BLOCKS(x)		((x & ~(MD_RESERVED_BLOCKS - 1)) - MD_RESERVED_BLOCKS)

#define MD_SB_BYTES			4096
#define MD_SB_WORDS			(MD_SB_BYTES / 4)
#define MD_SB_BLOCKS			(MD_SB_BYTES / BLOCK_SIZE)
#define MD_SB_SECTORS			(MD_SB_BYTES / 512)

/*
 * The following are counted in 32-bit words
 */
#define	MD_SB_GENERIC_OFFSET		0
#define MD_SB_PERSONALITY_OFFSET	64
#define MD_SB_DISKS_OFFSET		128
#define MD_SB_DESCRIPTOR_OFFSET		992

#define MD_SB_GENERIC_CONSTANT_WORDS	32
#define MD_SB_GENERIC_STATE_WORDS	32
#define MD_SB_GENERIC_WORDS		(MD_SB_GENERIC_CONSTANT_WORDS + MD_SB_GENERIC_STATE_WORDS)
#define MD_SB_PERSONALITY_WORDS		64
#define MD_SB_DESCRIPTOR_WORDS		32
#define MD_SB_DISKS			27
#define MD_SB_DISKS_WORDS		(MD_SB_DISKS*MD_SB_DESCRIPTOR_WORDS)
#define MD_SB_RESERVED_WORDS		(1024 - MD_SB_GENERIC_WORDS - MD_SB_PERSONALITY_WORDS - MD_SB_DISKS_WORDS - MD_SB_DESCRIPTOR_WORDS)
#define MD_SB_EQUAL_WORDS		(MD_SB_GENERIC_WORDS + MD_SB_PERSONALITY_WORDS + MD_SB_DISKS_WORDS)

/*
 * Device "operational" state bits
 */
#define MD_DISK_FAULTY		0 /* disk is faulty / operational */
#define MD_DISK_ACTIVE		1 /* disk is running but may not be in sync */
#define MD_DISK_SYNC		2 /* disk is in sync with the raid set */
#define MD_DISK_REMOVED		3 /* disk is in sync with the raid set */
#define MD_DISK_CLUSTER_ADD	4 /* Initiate a disk add across the cluster
				   * For clustered enviroments only.
				   */
#define MD_DISK_CANDIDATE	5 /* disk is added as spare (local) until confirmed
				   * For clustered enviroments only.
				   */

#define	MD_DISK_WRITEMOSTLY	9 /* disk is "write-mostly" is RAID1 config.
				   * read requests will only be sent here in
				   * dire need
				   */
#define	MD_DISK_FAILFAST	10 /* Fewer retries, more failures */

#define MD_DISK_REPLACEMENT	17
#define MD_DISK_JOURNAL		18 /* disk is used as the write journal in RAID-5/6 */

#define MD_DISK_ROLE_SPARE	0xffff
#define MD_DISK_ROLE_FAULTY	0xfffe
#define MD_DISK_ROLE_JOURNAL	0xfffd
#define MD_DISK_ROLE_MAX	0xff00 /* max value of regular disk role */

typedef struct mdp_device_descriptor_s {
	__u32 number;		/* 0 Device number in the entire set	      */
	__u32 major;		/* 1 Device major number		      */
	__u32 minor;		/* 2 Device minor number		      */
	__u32 raid_disk;	/* 3 The role of the device in the raid set   */
	__u32 state;		/* 4 Operational state			      */
	__u32 reserved[MD_SB_DESCRIPTOR_WORDS - 5];
} mdp_disk_t;

#define MD_SB_MAGIC		0xa92b4efc

/*
 * Superblock state bits
 */
#define MD_SB_CLEAN		0
#define MD_SB_ERRORS		1
#define MD_SB_BBM_ERRORS	2
#define MD_SB_BLOCK_CONTAINER_RESHAPE 3 /* block container wide reshapes */
#define MD_SB_BLOCK_VOLUME	4 /* block activation of array, other arrays
				   * in container can be activated */
#define MD_SB_CLUSTERED		5 /* MD is clustered  */
#define	MD_SB_BITMAP_PRESENT	8 /* bitmap may be present nearby */

typedef struct mdp_superblock_s {
	/*
	 * Constant generic information
	 */
	__u32 md_magic;		/*  0 MD identifier			      */
	__u32 major_version;	/*  1 major version to which the set conforms */
	__u32 minor_version;	/*  2 minor version ...			      */
	__u32 patch_version;	/*  3 patchlevel version ...		      */
	__u32 gvalid_words;	/*  4 Number of used words in this section    */
	__u32 set_uuid0;	/*  5 Raid set identifier		      */
	__u32 ctime;		/*  6 Creation time			      */
	__u32 level;		/*  7 Raid personality			      */
	__u32 size;		/*  8 Apparent size of each individual disk   */
	__u32 nr_disks;		/*  9 total disks in the raid set	      */
	__u32 raid_disks;	/* 10 disks in a fully functional raid set    */
	__u32 md_minor;		/* 11 preferred MD minor device number	      */
	__u32 not_persistent;	/* 12 does it have a persistent superblock    */
	__u32 set_uuid1;	/* 13 Raid set identifier #2		      */
	__u32 set_uuid2;	/* 14 Raid set identifier #3		      */
	__u32 set_uuid3;	/* 15 Raid set identifier #4		      */
	__u32 gstate_creserved[MD_SB_GENERIC_CONSTANT_WORDS - 16];

	/*
	 * Generic state information
	 */
	__u32 utime;		/*  0 Superblock update time		      */
	__u32 state;		/*  1 State bits (clean, ...)		      */
	__u32 active_disks;	/*  2 Number of currently active disks	      */
	__u32 working_disks;	/*  3 Number of working disks		      */
	__u32 failed_disks;	/*  4 Number of failed disks		      */
	__u32 spare_disks;	/*  5 Number of spare disks		      */
	__u32 sb_csum;		/*  6 checksum of the whole superblock        */
#if  __BYTE_ORDER ==  __BIG_ENDIAN
	__u32 events_hi;	/*  7 high-order of superblock update count   */
	__u32 events_lo;	/*  8 low-order of superblock update count    */
	__u32 cp_events_hi;	/*  9 high-order of checkpoint update count   */
	__u32 cp_events_lo;	/* 10 low-order of checkpoint update count    */
#else
	__u32 events_lo;	/*  7 low-order of superblock update count    */
	__u32 events_hi;	/*  8 high-order of superblock update count   */
	__u32 cp_events_lo;	/*  9 low-order of checkpoint update count    */
	__u32 cp_events_hi;	/* 10 high-order of checkpoint update count   */
#endif
	__u32 recovery_cp;	/* 11 recovery checkpoint sector count	      */
	/* There are only valid for minor_version > 90 */
	__u64 reshape_position;	/* 12,13 next address in array-space for reshape */
	__u32 new_level;	/* 14 new level we are reshaping to	      */
	__u32 delta_disks;	/* 15 change in number of raid_disks	      */
	__u32 new_layout;	/* 16 new layout			      */
	__u32 new_chunk;	/* 17 new chunk size (bytes)		      */
	__u32 gstate_sreserved[MD_SB_GENERIC_STATE_WORDS - 18];

	/*
	 * Personality information
	 */
	__u32 layout;		/*  0 the array's physical layout	      */
	__u32 chunk_size;	/*  1 chunk size in bytes		      */
	__u32 root_pv;		/*  2 LV root PV */
	__u32 root_block;	/*  3 LV root block */
	__u32 pstate_reserved[MD_SB_PERSONALITY_WORDS - 4];

	/*
	 * Disks information
	 */
	mdp_disk_t disks[MD_SB_DISKS];

	/*
	 * Reserved
	 */
	__u32 reserved[MD_SB_RESERVED_WORDS];

	/*
	 * Active descriptor
	 */
	mdp_disk_t this_disk;

} mdp_super_t;

#ifdef __TINYC__
typedef unsigned long long __u64;
#endif

static inline __u64 md_event(mdp_super_t *sb) {
	__u64 ev = sb->events_hi;
	return (ev<<32)| sb->events_lo;
}

struct r5l_payload_header {
	__u16 type;
	__u16 flags;
} __attribute__ ((__packed__));

enum r5l_payload_type {
	R5LOG_PAYLOAD_DATA = 0,
	R5LOG_PAYLOAD_PARITY = 1,
	R5LOG_PAYLOAD_FLUSH = 2,
};

struct r5l_payload_data_parity {
	struct r5l_payload_header header;
	__u32 size; /* sector. data/parity size. each 4k has a checksum */
	__u64 location; /* sector. For data, it's raid sector. For
				parity, it's stripe sector */
	__u32 checksum[];
} __attribute__ ((__packed__));

enum r5l_payload_data_parity_flag {
	R5LOG_PAYLOAD_FLAG_DISCARD = 1, /* payload is discard */
	/*
	 * RESHAPED/RESHAPING is only set when there is reshape activity. Note,
	 * both data/parity of a stripe should have the same flag set
	 *
	 * RESHAPED: reshape is running, and this stripe finished reshape
	 * RESHAPING: reshape is running, and this stripe isn't reshaped
	 * */
	R5LOG_PAYLOAD_FLAG_RESHAPED = 2,
	R5LOG_PAYLOAD_FLAG_RESHAPING = 3,
};

struct r5l_payload_flush {
	struct r5l_payload_header header;
	__u32 size; /* flush_stripes size, bytes */
	__u64 flush_stripes[];
} __attribute__ ((__packed__));

enum r5l_payload_flush_flag {
	R5LOG_PAYLOAD_FLAG_FLUSH_STRIPE = 1, /* data represents whole stripe */
};

struct r5l_meta_block {
	__u32 magic;
	__u32 checksum;
	__u8 version;
	__u8 __zero_pading_1;
	__u16 __zero_pading_2;
	__u32 meta_size; /* whole size of the block */

	__u64 seq;
	__u64 position; /* sector, start from rdev->data_offset, current position */
	struct r5l_payload_header payloads[];
} __attribute__ ((__packed__));

#define R5LOG_VERSION 0x1
#define R5LOG_MAGIC 0x6433c509

struct ppl_header_entry {
	__u64 data_sector;	/* raid sector of the new data */
	__u32 pp_size;		/* length of partial parity */
	__u32 data_size;	/* length of data */
	__u32 parity_disk;	/* member disk containing parity */
	__u32 checksum;		/* checksum of this entry's partial parity */
} __attribute__ ((__packed__));

#define PPL_HEADER_SIZE 4096
#define PPL_HDR_RESERVED 512
#define PPL_HDR_ENTRY_SPACE \
	(PPL_HEADER_SIZE - PPL_HDR_RESERVED - 4 * sizeof(__u32) - sizeof(__u64))
#define PPL_HDR_MAX_ENTRIES \
	(PPL_HDR_ENTRY_SPACE / sizeof(struct ppl_header_entry))

struct ppl_header {
	__u8 reserved[PPL_HDR_RESERVED];/* reserved space, fill with 0xff */
	__u32 signature;		/* signature (family number of volume) */
	__u32 padding;			/* zero pad */
	__u64 generation;		/* generation number of the header */
	__u32 entries_count;		/* number of entries in entry array */
	__u32 checksum;			/* checksum of the header */
	struct ppl_header_entry entries[PPL_HDR_MAX_ENTRIES];
} __attribute__ ((__packed__));

#endif
