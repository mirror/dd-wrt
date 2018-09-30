/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2010 Neil Brown <neilb@suse.de>
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
 *    Email: <neil@brown.name>
 *
 */

/* Structure definitions ext  for MBR and GPT partition tables
 */

#define	MBR_SIGNATURE_MAGIC	__cpu_to_le16(0xAA55)
#define MBR_PARTITIONS               4

struct MBR_part_record {
  __u8 bootable;
  __u8 first_head;
  __u8 first_sector;
  __u8 first_cyl;
  __u8 part_type;
  __u8 last_head;
  __u8 last_sector;
  __u8 last_cyl;
  __u32 first_sect_lba;
  __u32 blocks_num;
} __attribute__((packed));

struct MBR {
	__u8 pad[446];
	struct MBR_part_record parts[MBR_PARTITIONS];
	__u16 magic;
} __attribute__((packed));

#define	GPT_SIGNATURE_MAGIC	__cpu_to_le64(0x5452415020494645ULL)
#define MBR_GPT_PARTITION_TYPE       0xEE

struct GPT_part_entry {
	unsigned char type_guid[16];
	unsigned char partition_guid[16];
	__u64 starting_lba;
	__u64 ending_lba;
	unsigned char attr_bits[8];
	unsigned char name[72];
} __attribute__((packed));

struct GPT {
	__u64 magic;
	__u32 revision;
	__u32 header_size;
	__u32 crc;
	__u32 pad1;
	__u64 current_lba;
	__u64 backup_lba;
	__u64 first_lba;
	__u64 last_lba;
	__u8 guid[16];
	__u64 part_start;
	__u32 part_cnt;
	__u32 part_size;
	__u32 part_crc;
	__u8 pad2[420];
} __attribute__((packed));
