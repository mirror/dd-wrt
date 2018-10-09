/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2006-2014 Neil Brown <neilb@suse.de>
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
 * Specifications for DDF taken from Common RAID DDF Specification Revision 1.2
 * (July 28 2006).  Reused by permission of SNIA.
 */

#define HAVE_STDINT_H 1
#include "mdadm.h"
#include "mdmon.h"
#include "sha1.h"
#include <values.h>
#include <stddef.h>

/* a non-official T10 name for creation GUIDs */
static char T10[] = "Linux-MD";

/* DDF timestamps are 1980 based, so we need to add
 * second-in-decade-of-seventies to convert to linux timestamps.
 * 10 years with 2 leap years.
 */
#define DECADE (3600*24*(365*10+2))
unsigned long crc32(
	unsigned long crc,
	const unsigned char *buf,
	unsigned len);

#define DDF_NOTFOUND (~0U)
#define DDF_CONTAINER (DDF_NOTFOUND-1)

/* Default for safe_mode_delay. Same value as for IMSM.
 */
static const int DDF_SAFE_MODE_DELAY = 4000;

/* The DDF metadata handling.
 * DDF metadata lives at the end of the device.
 * The last 512 byte block provides an 'anchor' which is used to locate
 * the rest of the metadata which usually lives immediately behind the anchor.
 *
 * Note:
 *  - all multibyte numeric fields are bigendian.
 *  - all strings are space padded.
 *
 */

typedef struct __be16 {
	__u16 _v16;
} be16;
#define be16_eq(x, y) ((x)._v16 == (y)._v16)
#define be16_and(x, y) ((x)._v16 & (y)._v16)
#define be16_or(x, y) ((x)._v16 | (y)._v16)
#define be16_clear(x, y) ((x)._v16 &= ~(y)._v16)
#define be16_set(x, y) ((x)._v16 |= (y)._v16)

typedef struct __be32 {
	__u32 _v32;
} be32;
#define be32_eq(x, y) ((x)._v32 == (y)._v32)

typedef struct __be64 {
	__u64 _v64;
} be64;
#define be64_eq(x, y) ((x)._v64 == (y)._v64)

#define be16_to_cpu(be) __be16_to_cpu((be)._v16)
static inline be16 cpu_to_be16(__u16 x)
{
	be16 be = { ._v16 = __cpu_to_be16(x) };
	return be;
}

#define be32_to_cpu(be) __be32_to_cpu((be)._v32)
static inline be32 cpu_to_be32(__u32 x)
{
	be32 be = { ._v32 = __cpu_to_be32(x) };
	return be;
}

#define be64_to_cpu(be) __be64_to_cpu((be)._v64)
static inline be64 cpu_to_be64(__u64 x)
{
	be64 be = { ._v64 = __cpu_to_be64(x) };
	return be;
}

/* Primary Raid Level (PRL) */
#define	DDF_RAID0	0x00
#define	DDF_RAID1	0x01
#define	DDF_RAID3	0x03
#define	DDF_RAID4	0x04
#define	DDF_RAID5	0x05
#define	DDF_RAID1E	0x11
#define	DDF_JBOD	0x0f
#define	DDF_CONCAT	0x1f
#define	DDF_RAID5E	0x15
#define	DDF_RAID5EE	0x25
#define	DDF_RAID6	0x06

/* Raid Level Qualifier (RLQ) */
#define	DDF_RAID0_SIMPLE	0x00
#define	DDF_RAID1_SIMPLE	0x00 /* just 2 devices in this plex */
#define	DDF_RAID1_MULTI		0x01 /* exactly 3 devices in this plex */
#define	DDF_RAID3_0		0x00 /* parity in first extent */
#define	DDF_RAID3_N		0x01 /* parity in last extent */
#define	DDF_RAID4_0		0x00 /* parity in first extent */
#define	DDF_RAID4_N		0x01 /* parity in last extent */
/* these apply to raid5e and raid5ee as well */
#define	DDF_RAID5_0_RESTART	0x00 /* same as 'right asymmetric' - layout 1 */
#define	DDF_RAID6_0_RESTART	0x01 /* raid6 different from raid5 here!!! */
#define	DDF_RAID5_N_RESTART	0x02 /* same as 'left asymmetric' - layout 0 */
#define	DDF_RAID5_N_CONTINUE	0x03 /* same as 'left symmetric' - layout 2 */

#define	DDF_RAID1E_ADJACENT	0x00 /* raid10 nearcopies==2 */
#define	DDF_RAID1E_OFFSET	0x01 /* raid10 offsetcopies==2 */

/* Secondary RAID Level (SRL) */
#define	DDF_2STRIPED	0x00	/* This is weirder than RAID0 !! */
#define	DDF_2MIRRORED	0x01
#define	DDF_2CONCAT	0x02
#define	DDF_2SPANNED	0x03	/* This is also weird - be careful */

/* Magic numbers */
#define	DDF_HEADER_MAGIC	cpu_to_be32(0xDE11DE11)
#define	DDF_CONTROLLER_MAGIC	cpu_to_be32(0xAD111111)
#define	DDF_PHYS_RECORDS_MAGIC	cpu_to_be32(0x22222222)
#define	DDF_PHYS_DATA_MAGIC	cpu_to_be32(0x33333333)
#define	DDF_VIRT_RECORDS_MAGIC	cpu_to_be32(0xDDDDDDDD)
#define	DDF_VD_CONF_MAGIC	cpu_to_be32(0xEEEEEEEE)
#define	DDF_SPARE_ASSIGN_MAGIC	cpu_to_be32(0x55555555)
#define	DDF_VU_CONF_MAGIC	cpu_to_be32(0x88888888)
#define	DDF_VENDOR_LOG_MAGIC	cpu_to_be32(0x01dBEEF0)
#define	DDF_BBM_LOG_MAGIC	cpu_to_be32(0xABADB10C)

#define	DDF_GUID_LEN	24
#define DDF_REVISION_0	"01.00.00"
#define DDF_REVISION_2	"01.02.00"

struct ddf_header {
	be32	magic;		/* DDF_HEADER_MAGIC */
	be32	crc;
	char	guid[DDF_GUID_LEN];
	char	revision[8];	/* 01.02.00 */
	be32	seq;		/* starts at '1' */
	be32	timestamp;
	__u8	openflag;
	__u8	foreignflag;
	__u8	enforcegroups;
	__u8	pad0;		/* 0xff */
	__u8	pad1[12];	/* 12 * 0xff */
	/* 64 bytes so far */
	__u8	header_ext[32];	/* reserved: fill with 0xff */
	be64	primary_lba;
	be64	secondary_lba;
	__u8	type;
	__u8	pad2[3];	/* 0xff */
	be32	workspace_len;	/* sectors for vendor space -
				 * at least 32768(sectors) */
	be64	workspace_lba;
	be16	max_pd_entries;	/* one of 15, 63, 255, 1023, 4095 */
	be16	max_vd_entries; /* 2^(4,6,8,10,12)-1 : i.e. as above */
	be16	max_partitions; /* i.e. max num of configuration
				   record entries per disk */
	be16	config_record_len; /* 1 +ROUNDUP(max_primary_element_entries
				                 *12/512) */
	be16	max_primary_element_entries; /* 16, 64, 256, 1024, or 4096 */
	__u8	pad3[54];	/* 0xff */
	/* 192 bytes so far */
	be32	controller_section_offset;
	be32	controller_section_length;
	be32	phys_section_offset;
	be32	phys_section_length;
	be32	virt_section_offset;
	be32	virt_section_length;
	be32	config_section_offset;
	be32	config_section_length;
	be32	data_section_offset;
	be32	data_section_length;
	be32	bbm_section_offset;
	be32	bbm_section_length;
	be32	diag_space_offset;
	be32	diag_space_length;
	be32	vendor_offset;
	be32	vendor_length;
	/* 256 bytes so far */
	__u8	pad4[256];	/* 0xff */
};

/* type field */
#define	DDF_HEADER_ANCHOR	0x00
#define	DDF_HEADER_PRIMARY	0x01
#define	DDF_HEADER_SECONDARY	0x02

/* The content of the 'controller section' - global scope */
struct ddf_controller_data {
	be32	magic;			/* DDF_CONTROLLER_MAGIC */
	be32	crc;
	char	guid[DDF_GUID_LEN];
	struct controller_type {
		be16 vendor_id;
		be16 device_id;
		be16 sub_vendor_id;
		be16 sub_device_id;
	} type;
	char	product_id[16];
	__u8	pad[8];	/* 0xff */
	__u8	vendor_data[448];
};

/* The content of phys_section - global scope */
struct phys_disk {
	be32	magic;		/* DDF_PHYS_RECORDS_MAGIC */
	be32	crc;
	be16	used_pdes;	/* This is a counter, not a max - the list
				 * of used entries may not be dense */
	be16	max_pdes;
	__u8	pad[52];
	struct phys_disk_entry {
		char	guid[DDF_GUID_LEN];
		be32	refnum;
		be16	type;
		be16	state;
		be64	config_size;	/* DDF structures must be after here */
		char	path[18];	/* Another horrible structure really
					 * but is "used for information
					 * purposes only" */
		__u8	pad[6];
	} entries[0];
};

/* phys_disk_entry.type is a bitmap - bigendian remember */
#define	DDF_Forced_PD_GUID		1
#define	DDF_Active_in_VD		2
#define	DDF_Global_Spare		4 /* VD_CONF records are ignored */
#define	DDF_Spare			8 /* overrides Global_spare */
#define	DDF_Foreign			16
#define	DDF_Legacy			32 /* no DDF on this device */

#define	DDF_Interface_mask		0xf00
#define	DDF_Interface_SCSI		0x100
#define	DDF_Interface_SAS		0x200
#define	DDF_Interface_SATA		0x300
#define	DDF_Interface_FC		0x400

/* phys_disk_entry.state is a bigendian bitmap */
#define	DDF_Online			1
#define	DDF_Failed			2 /* overrides  1,4,8 */
#define	DDF_Rebuilding			4
#define	DDF_Transition			8
#define	DDF_SMART			16
#define	DDF_ReadErrors			32
#define	DDF_Missing			64

/* The content of the virt_section global scope */
struct virtual_disk {
	be32	magic;		/* DDF_VIRT_RECORDS_MAGIC */
	be32	crc;
	be16	populated_vdes;
	be16	max_vdes;
	__u8	pad[52];
	struct virtual_entry {
		char	guid[DDF_GUID_LEN];
		be16	unit;
		__u16	pad0;	/* 0xffff */
		be16	guid_crc;
		be16	type;
		__u8	state;
		__u8	init_state;
		__u8	pad1[14];
		char	name[16];
	} entries[0];
};

/* virtual_entry.type is a bitmap - bigendian */
#define	DDF_Shared		1
#define	DDF_Enforce_Groups	2
#define	DDF_Unicode		4
#define	DDF_Owner_Valid		8

/* virtual_entry.state is a bigendian bitmap */
#define	DDF_state_mask		0x7
#define	DDF_state_optimal	0x0
#define	DDF_state_degraded	0x1
#define	DDF_state_deleted	0x2
#define	DDF_state_missing	0x3
#define	DDF_state_failed	0x4
#define	DDF_state_part_optimal	0x5

#define	DDF_state_morphing	0x8
#define	DDF_state_inconsistent	0x10

/* virtual_entry.init_state is a bigendian bitmap */
#define	DDF_initstate_mask	0x03
#define	DDF_init_not		0x00
#define	DDF_init_quick		0x01 /* initialisation is progress.
				      * i.e. 'state_inconsistent' */
#define	DDF_init_full		0x02

#define	DDF_access_mask		0xc0
#define	DDF_access_rw		0x00
#define	DDF_access_ro		0x80
#define	DDF_access_blocked	0xc0

/* The content of the config_section - local scope
 * It has multiple records each config_record_len sectors
 * They can be vd_config or spare_assign
 */

struct vd_config {
	be32	magic;		/* DDF_VD_CONF_MAGIC */
	be32	crc;
	char	guid[DDF_GUID_LEN];
	be32	timestamp;
	be32	seqnum;
	__u8	pad0[24];
	be16	prim_elmnt_count;
	__u8	chunk_shift;	/* 0 == 512, 1==1024 etc */
	__u8	prl;
	__u8	rlq;
	__u8	sec_elmnt_count;
	__u8	sec_elmnt_seq;
	__u8	srl;
	be64	blocks;		/* blocks per component could be different
				 * on different component devices...(only
				 * for concat I hope) */
	be64	array_blocks;	/* blocks in array */
	__u8	pad1[8];
	be32	spare_refs[8];	/* This is used to detect missing spares.
				 * As we don't have an interface for that
				 * the values are ignored.
				 */
	__u8	cache_pol[8];
	__u8	bg_rate;
	__u8	pad2[3];
	__u8	pad3[52];
	__u8	pad4[192];
	__u8	v0[32];	/* reserved- 0xff */
	__u8	v1[32];	/* reserved- 0xff */
	__u8	v2[16];	/* reserved- 0xff */
	__u8	v3[16];	/* reserved- 0xff */
	__u8	vendor[32];
	be32	phys_refnum[0];	/* refnum of each disk in sequence */
      /*__u64	lba_offset[0];  LBA offset in each phys.  Note extents in a
				bvd are always the same size */
};
#define LBA_OFFSET(ddf, vd) ((be64 *) &(vd)->phys_refnum[(ddf)->mppe])

/* vd_config.cache_pol[7] is a bitmap */
#define	DDF_cache_writeback	1	/* else writethrough */
#define	DDF_cache_wadaptive	2	/* only applies if writeback */
#define	DDF_cache_readahead	4
#define	DDF_cache_radaptive	8	/* only if doing read-ahead */
#define	DDF_cache_ifnobatt	16	/* even to write cache if battery is poor */
#define	DDF_cache_wallowed	32	/* enable write caching */
#define	DDF_cache_rallowed	64	/* enable read caching */

struct spare_assign {
	be32	magic;		/* DDF_SPARE_ASSIGN_MAGIC */
	be32	crc;
	be32	timestamp;
	__u8	reserved[7];
	__u8	type;
	be16	populated;	/* SAEs used */
	be16	max;		/* max SAEs */
	__u8	pad[8];
	struct spare_assign_entry {
		char	guid[DDF_GUID_LEN];
		be16	secondary_element;
		__u8	pad[6];
	} spare_ents[0];
};
/* spare_assign.type is a bitmap */
#define	DDF_spare_dedicated	0x1	/* else global */
#define	DDF_spare_revertible	0x2	/* else committable */
#define	DDF_spare_active	0x4	/* else not active */
#define	DDF_spare_affinity	0x8	/* enclosure affinity */

/* The data_section contents - local scope */
struct disk_data {
	be32	magic;		/* DDF_PHYS_DATA_MAGIC */
	be32	crc;
	char	guid[DDF_GUID_LEN];
	be32	refnum;		/* crc of some magic drive data ... */
	__u8	forced_ref;	/* set when above was not result of magic */
	__u8	forced_guid;	/* set if guid was forced rather than magic */
	__u8	vendor[32];
	__u8	pad[442];
};

/* bbm_section content */
struct bad_block_log {
	be32	magic;
	be32	crc;
	be16	entry_count;
	be32	spare_count;
	__u8	pad[10];
	be64	first_spare;
	struct mapped_block {
		be64	defective_start;
		be32	replacement_start;
		be16	remap_count;
		__u8	pad[2];
	} entries[0];
};

/* Struct for internally holding ddf structures */
/* The DDF structure stored on each device is potentially
 * quite different, as some data is global and some is local.
 * The global data is:
 *   - ddf header
 *   - controller_data
 *   - Physical disk records
 *   - Virtual disk records
 * The local data is:
 *   - Configuration records
 *   - Physical Disk data section
 *  (  and Bad block and vendor which I don't care about yet).
 *
 * The local data is parsed into separate lists as it is read
 * and reconstructed for writing.  This means that we only need
 * to make config changes once and they are automatically
 * propagated to all devices.
 * The global (config and disk data) records are each in a list
 * of separate data structures.  When writing we find the entry
 * or entries applicable to the particular device.
 */
struct ddf_super {
	struct ddf_header	anchor, primary, secondary;
	struct ddf_controller_data controller;
	struct ddf_header	*active;
	struct phys_disk	*phys;
	struct virtual_disk	*virt;
	char			*conf;
	int			pdsize, vdsize;
	unsigned int		max_part, mppe, conf_rec_len;
	int			currentdev;
	int			updates_pending;
	struct vcl {
		union {
			char space[512];
			struct {
				struct vcl	*next;
				unsigned int	vcnum; /* index into ->virt */
				/* For an array with a secondary level there are
				 * multiple vd_config structures, all with the same
				 * guid but with different sec_elmnt_seq.
				 * One of these structures is in 'conf' below.
				 * The others are in other_bvds, not in any
				 * particular order.
				 */
				struct vd_config **other_bvds;
				__u64		*block_sizes; /* NULL if all the same */
			};
		};
		struct vd_config conf;
	} *conflist, *currentconf;
	struct dl {
		union {
			char space[512];
			struct {
				struct dl	*next;
				int major, minor;
				char *devname;
				int fd;
				unsigned long long size; /* sectors */
				be64 primary_lba; /* sectors */
				be64 secondary_lba; /* sectors */
				be64 workspace_lba; /* sectors */
				int pdnum;	/* index in ->phys */
				struct spare_assign *spare;
				void *mdupdate; /* hold metadata update */

				/* These fields used by auto-layout */
				int raiddisk; /* slot to fill in autolayout */
				__u64 esize;
				int displayed;
			};
		};
		struct disk_data disk;
		struct vcl *vlist[0]; /* max_part in size */
	} *dlist, *add_list;
};

static int load_super_ddf_all(struct supertype *st, int fd,
			      void **sbp, char *devname);
static int get_svd_state(const struct ddf_super *, const struct vcl *);
static int
validate_geometry_ddf_container(struct supertype *st,
				int level, int layout, int raiddisks,
				int chunk, unsigned long long size,
				unsigned long long data_offset,
				char *dev, unsigned long long *freesize,
				int verbose);

static int validate_geometry_ddf_bvd(struct supertype *st,
				     int level, int layout, int raiddisks,
				     int *chunk, unsigned long long size,
				     unsigned long long data_offset,
				     char *dev, unsigned long long *freesize,
				     int verbose);

static void free_super_ddf(struct supertype *st);
static int all_ff(const char *guid);
static unsigned int get_pd_index_from_refnum(const struct vcl *vc,
					     be32 refnum, unsigned int nmax,
					     const struct vd_config **bvd,
					     unsigned int *idx);
static void getinfo_super_ddf(struct supertype *st, struct mdinfo *info, char *map);
static void uuid_from_ddf_guid(const char *guid, int uuid[4]);
static void uuid_from_super_ddf(struct supertype *st, int uuid[4]);
static void _ddf_array_name(char *name, const struct ddf_super *ddf, int i);
static void getinfo_super_ddf_bvd(struct supertype *st, struct mdinfo *info, char *map);
static int init_super_ddf_bvd(struct supertype *st,
			      mdu_array_info_t *info,
			      unsigned long long size,
			      char *name, char *homehost,
			      int *uuid, unsigned long long data_offset);

#if DEBUG
static void pr_state(struct ddf_super *ddf, const char *msg)
{
	unsigned int i;
	dprintf("%s: ", msg);
	for (i = 0; i < be16_to_cpu(ddf->active->max_vd_entries); i++) {
		if (all_ff(ddf->virt->entries[i].guid))
			continue;
		dprintf_cont("%u(s=%02x i=%02x) ", i,
			ddf->virt->entries[i].state,
			ddf->virt->entries[i].init_state);
	}
	dprintf_cont("\n");
}
#else
static void pr_state(const struct ddf_super *ddf, const char *msg) {}
#endif

static void _ddf_set_updates_pending(struct ddf_super *ddf, struct vd_config *vc,
				     const char *func)
{
	if (vc) {
		vc->timestamp = cpu_to_be32(time(0)-DECADE);
		vc->seqnum = cpu_to_be32(be32_to_cpu(vc->seqnum) + 1);
	}
	if (ddf->updates_pending)
		return;
	ddf->updates_pending = 1;
	ddf->active->seq = cpu_to_be32((be32_to_cpu(ddf->active->seq)+1));
	pr_state(ddf, func);
}

#define ddf_set_updates_pending(x,v) _ddf_set_updates_pending((x), (v), __func__)

static be32 calc_crc(void *buf, int len)
{
	/* crcs are always at the same place as in the ddf_header */
	struct ddf_header *ddf = buf;
	be32 oldcrc = ddf->crc;
	__u32 newcrc;
	ddf->crc = cpu_to_be32(0xffffffff);

	newcrc = crc32(0, buf, len);
	ddf->crc = oldcrc;
	/* The crc is stored (like everything) bigendian, so convert
	 * here for simplicity
	 */
	return cpu_to_be32(newcrc);
}

#define DDF_INVALID_LEVEL 0xff
#define DDF_NO_SECONDARY 0xff
static int err_bad_md_layout(const mdu_array_info_t *array)
{
	pr_err("RAID%d layout %x with %d disks is unsupported for DDF\n",
	       array->level, array->layout, array->raid_disks);
	return -1;
}

static int layout_md2ddf(const mdu_array_info_t *array,
			 struct vd_config *conf)
{
	be16 prim_elmnt_count = cpu_to_be16(array->raid_disks);
	__u8 prl = DDF_INVALID_LEVEL, rlq = 0;
	__u8 sec_elmnt_count = 1;
	__u8 srl = DDF_NO_SECONDARY;

	switch (array->level) {
	case LEVEL_LINEAR:
		prl = DDF_CONCAT;
		break;
	case 0:
		rlq = DDF_RAID0_SIMPLE;
		prl = DDF_RAID0;
		break;
	case 1:
		switch (array->raid_disks) {
		case 2:
			rlq = DDF_RAID1_SIMPLE;
			break;
		case 3:
			rlq = DDF_RAID1_MULTI;
			break;
		default:
			return err_bad_md_layout(array);
		}
		prl = DDF_RAID1;
		break;
	case 4:
		if (array->layout != 0)
			return err_bad_md_layout(array);
		rlq = DDF_RAID4_N;
		prl = DDF_RAID4;
		break;
	case 5:
		switch (array->layout) {
		case ALGORITHM_LEFT_ASYMMETRIC:
			rlq = DDF_RAID5_N_RESTART;
			break;
		case ALGORITHM_RIGHT_ASYMMETRIC:
			rlq = DDF_RAID5_0_RESTART;
			break;
		case ALGORITHM_LEFT_SYMMETRIC:
			rlq = DDF_RAID5_N_CONTINUE;
			break;
		case ALGORITHM_RIGHT_SYMMETRIC:
			/* not mentioned in standard */
		default:
			return err_bad_md_layout(array);
		}
		prl = DDF_RAID5;
		break;
	case 6:
		switch (array->layout) {
		case ALGORITHM_ROTATING_N_RESTART:
			rlq = DDF_RAID5_N_RESTART;
			break;
		case ALGORITHM_ROTATING_ZERO_RESTART:
			rlq = DDF_RAID6_0_RESTART;
			break;
		case ALGORITHM_ROTATING_N_CONTINUE:
			rlq = DDF_RAID5_N_CONTINUE;
			break;
		default:
			return err_bad_md_layout(array);
		}
		prl = DDF_RAID6;
		break;
	case 10:
		if (array->raid_disks % 2 == 0 && array->layout == 0x102) {
			rlq = DDF_RAID1_SIMPLE;
			prim_elmnt_count =  cpu_to_be16(2);
			sec_elmnt_count = array->raid_disks / 2;
			srl = DDF_2SPANNED;
			prl = DDF_RAID1;
		} else if (array->raid_disks % 3 == 0 &&
			   array->layout == 0x103) {
			rlq = DDF_RAID1_MULTI;
			prim_elmnt_count =  cpu_to_be16(3);
			sec_elmnt_count = array->raid_disks / 3;
			srl = DDF_2SPANNED;
			prl = DDF_RAID1;
		} else if (array->layout == 0x201) {
			prl = DDF_RAID1E;
			rlq = DDF_RAID1E_OFFSET;
		} else if (array->layout == 0x102) {
			prl = DDF_RAID1E;
			rlq = DDF_RAID1E_ADJACENT;
		} else
			return err_bad_md_layout(array);
		break;
	default:
		return err_bad_md_layout(array);
	}
	conf->prl = prl;
	conf->prim_elmnt_count = prim_elmnt_count;
	conf->rlq = rlq;
	conf->srl = srl;
	conf->sec_elmnt_count = sec_elmnt_count;
	return 0;
}

static int err_bad_ddf_layout(const struct vd_config *conf)
{
	pr_err("DDF RAID %u qualifier %u with %u disks is unsupported\n",
	       conf->prl, conf->rlq, be16_to_cpu(conf->prim_elmnt_count));
	return -1;
}

static int layout_ddf2md(const struct vd_config *conf,
			 mdu_array_info_t *array)
{
	int level = LEVEL_UNSUPPORTED;
	int layout = 0;
	int raiddisks = be16_to_cpu(conf->prim_elmnt_count);

	if (conf->sec_elmnt_count > 1) {
		/* see also check_secondary() */
		if (conf->prl != DDF_RAID1 ||
		    (conf->srl != DDF_2STRIPED && conf->srl != DDF_2SPANNED)) {
			pr_err("Unsupported secondary RAID level %u/%u\n",
			       conf->prl, conf->srl);
			return -1;
		}
		if (raiddisks == 2 && conf->rlq == DDF_RAID1_SIMPLE)
			layout = 0x102;
		else if  (raiddisks == 3 && conf->rlq == DDF_RAID1_MULTI)
			layout = 0x103;
		else
			return err_bad_ddf_layout(conf);
		raiddisks *= conf->sec_elmnt_count;
		level = 10;
		goto good;
	}

	switch (conf->prl) {
	case DDF_CONCAT:
		level = LEVEL_LINEAR;
		break;
	case DDF_RAID0:
		if (conf->rlq != DDF_RAID0_SIMPLE)
			return err_bad_ddf_layout(conf);
		level = 0;
		break;
	case DDF_RAID1:
		if (!((conf->rlq == DDF_RAID1_SIMPLE && raiddisks == 2) ||
		      (conf->rlq == DDF_RAID1_MULTI && raiddisks == 3)))
			return err_bad_ddf_layout(conf);
		level = 1;
		break;
	case DDF_RAID1E:
		if (conf->rlq == DDF_RAID1E_ADJACENT)
			layout = 0x102;
		else if (conf->rlq == DDF_RAID1E_OFFSET)
			layout = 0x201;
		else
			return err_bad_ddf_layout(conf);
		level = 10;
		break;
	case DDF_RAID4:
		if (conf->rlq != DDF_RAID4_N)
			return err_bad_ddf_layout(conf);
		level = 4;
		break;
	case DDF_RAID5:
		switch (conf->rlq) {
		case DDF_RAID5_N_RESTART:
			layout = ALGORITHM_LEFT_ASYMMETRIC;
			break;
		case DDF_RAID5_0_RESTART:
			layout = ALGORITHM_RIGHT_ASYMMETRIC;
			break;
		case DDF_RAID5_N_CONTINUE:
			layout = ALGORITHM_LEFT_SYMMETRIC;
			break;
		default:
			return err_bad_ddf_layout(conf);
		}
		level = 5;
		break;
	case DDF_RAID6:
		switch (conf->rlq) {
		case DDF_RAID5_N_RESTART:
			layout = ALGORITHM_ROTATING_N_RESTART;
			break;
		case DDF_RAID6_0_RESTART:
			layout = ALGORITHM_ROTATING_ZERO_RESTART;
			break;
		case DDF_RAID5_N_CONTINUE:
			layout = ALGORITHM_ROTATING_N_CONTINUE;
			break;
		default:
			return err_bad_ddf_layout(conf);
		}
		level = 6;
		break;
	default:
		return err_bad_ddf_layout(conf);
	};

good:
	array->level = level;
	array->layout = layout;
	array->raid_disks = raiddisks;
	return 0;
}

static int load_ddf_header(int fd, unsigned long long lba,
			   unsigned long long size,
			   int type,
			   struct ddf_header *hdr, struct ddf_header *anchor)
{
	/* read a ddf header (primary or secondary) from fd/lba
	 * and check that it is consistent with anchor
	 * Need to check:
	 *   magic, crc, guid, rev, and LBA's header_type, and
	 *  everything after header_type must be the same
	 */
	if (lba >= size-1)
		return 0;

	if (lseek64(fd, lba<<9, 0) < 0)
		return 0;

	if (read(fd, hdr, 512) != 512)
		return 0;

	if (!be32_eq(hdr->magic, DDF_HEADER_MAGIC)) {
		pr_err("bad header magic\n");
		return 0;
	}
	if (!be32_eq(calc_crc(hdr, 512), hdr->crc)) {
		pr_err("bad CRC\n");
		return 0;
	}
	if (memcmp(anchor->guid, hdr->guid, DDF_GUID_LEN) != 0 ||
	    memcmp(anchor->revision, hdr->revision, 8) != 0 ||
	    !be64_eq(anchor->primary_lba, hdr->primary_lba) ||
	    !be64_eq(anchor->secondary_lba, hdr->secondary_lba) ||
	    hdr->type != type ||
	    memcmp(anchor->pad2, hdr->pad2, 512 -
		   offsetof(struct ddf_header, pad2)) != 0) {
		pr_err("header mismatch\n");
		return 0;
	}

	/* Looks good enough to me... */
	return 1;
}

static void *load_section(int fd, struct ddf_super *super, void *buf,
			  be32 offset_be, be32 len_be, int check)
{
	unsigned long long offset = be32_to_cpu(offset_be);
	unsigned long long len = be32_to_cpu(len_be);
	int dofree = (buf == NULL);

	if (check)
		if (len != 2 && len != 8 && len != 32 &&
		    len != 128 && len != 512)
			return NULL;

	if (len > 1024)
		return NULL;
	if (!buf && posix_memalign(&buf, 512, len<<9) != 0)
		buf = NULL;

	if (!buf)
		return NULL;

	if (super->active->type == 1)
		offset += be64_to_cpu(super->active->primary_lba);
	else
		offset += be64_to_cpu(super->active->secondary_lba);

	if ((unsigned long long)lseek64(fd, offset<<9, 0) != (offset<<9)) {
		if (dofree)
			free(buf);
		return NULL;
	}
	if ((unsigned long long)read(fd, buf, len<<9) != (len<<9)) {
		if (dofree)
			free(buf);
		return NULL;
	}
	return buf;
}

static int load_ddf_headers(int fd, struct ddf_super *super, char *devname)
{
	unsigned long long dsize;

	get_dev_size(fd, NULL, &dsize);

	if (lseek64(fd, dsize-512, 0) < 0) {
		if (devname)
			pr_err("Cannot seek to anchor block on %s: %s\n",
			       devname, strerror(errno));
		return 1;
	}
	if (read(fd, &super->anchor, 512) != 512) {
		if (devname)
			pr_err("Cannot read anchor block on %s: %s\n",
			       devname, strerror(errno));
		return 1;
	}
	if (!be32_eq(super->anchor.magic, DDF_HEADER_MAGIC)) {
		if (devname)
			pr_err("no DDF anchor found on %s\n",
				devname);
		return 2;
	}
	if (!be32_eq(calc_crc(&super->anchor, 512), super->anchor.crc)) {
		if (devname)
			pr_err("bad CRC on anchor on %s\n",
				devname);
		return 2;
	}
	if (memcmp(super->anchor.revision, DDF_REVISION_0, 8) != 0 &&
	    memcmp(super->anchor.revision, DDF_REVISION_2, 8) != 0) {
		if (devname)
			pr_err("can only support super revision %.8s and earlier, not %.8s on %s\n",
				DDF_REVISION_2, super->anchor.revision,devname);
		return 2;
	}
	super->active = NULL;
	if (load_ddf_header(fd, be64_to_cpu(super->anchor.primary_lba),
			    dsize >> 9,  1,
			    &super->primary, &super->anchor) == 0) {
		if (devname)
			pr_err("Failed to load primary DDF header on %s\n", devname);
	} else
		super->active = &super->primary;

	if (load_ddf_header(fd, be64_to_cpu(super->anchor.secondary_lba),
			    dsize >> 9,  2,
			    &super->secondary, &super->anchor)) {
		if (super->active == NULL ||
		    (be32_to_cpu(super->primary.seq)
		     < be32_to_cpu(super->secondary.seq) &&
			!super->secondary.openflag) ||
		    (be32_to_cpu(super->primary.seq) ==
		     be32_to_cpu(super->secondary.seq) &&
			super->primary.openflag && !super->secondary.openflag))
			super->active = &super->secondary;
	} else if (devname &&
		   be64_to_cpu(super->anchor.secondary_lba) != ~(__u64)0)
		pr_err("Failed to load secondary DDF header on %s\n",
		       devname);
	if (super->active == NULL)
		return 2;
	return 0;
}

static int load_ddf_global(int fd, struct ddf_super *super, char *devname)
{
	void *ok;
	ok = load_section(fd, super, &super->controller,
			  super->active->controller_section_offset,
			  super->active->controller_section_length,
			  0);
	super->phys = load_section(fd, super, NULL,
				   super->active->phys_section_offset,
				   super->active->phys_section_length,
				   1);
	super->pdsize = be32_to_cpu(super->active->phys_section_length) * 512;

	super->virt = load_section(fd, super, NULL,
				   super->active->virt_section_offset,
				   super->active->virt_section_length,
				   1);
	super->vdsize = be32_to_cpu(super->active->virt_section_length) * 512;
	if (!ok ||
	    !super->phys ||
	    !super->virt) {
		free(super->phys);
		free(super->virt);
		super->phys = NULL;
		super->virt = NULL;
		return 2;
	}
	super->conflist = NULL;
	super->dlist = NULL;

	super->max_part = be16_to_cpu(super->active->max_partitions);
	super->mppe = be16_to_cpu(super->active->max_primary_element_entries);
	super->conf_rec_len = be16_to_cpu(super->active->config_record_len);
	return 0;
}

#define DDF_UNUSED_BVD 0xff
static int alloc_other_bvds(const struct ddf_super *ddf, struct vcl *vcl)
{
	unsigned int n_vds = vcl->conf.sec_elmnt_count - 1;
	unsigned int i, vdsize;
	void *p;
	if (n_vds == 0) {
		vcl->other_bvds = NULL;
		return 0;
	}
	vdsize = ddf->conf_rec_len * 512;
	if (posix_memalign(&p, 512, n_vds *
			   (vdsize +  sizeof(struct vd_config *))) != 0)
		return -1;
	vcl->other_bvds = (struct vd_config **) (p + n_vds * vdsize);
	for (i = 0; i < n_vds; i++) {
		vcl->other_bvds[i] = p + i * vdsize;
		memset(vcl->other_bvds[i], 0, vdsize);
		vcl->other_bvds[i]->sec_elmnt_seq = DDF_UNUSED_BVD;
	}
	return 0;
}

static void add_other_bvd(struct vcl *vcl, struct vd_config *vd,
			  unsigned int len)
{
	int i;
	for (i = 0; i < vcl->conf.sec_elmnt_count-1; i++)
		if (vcl->other_bvds[i]->sec_elmnt_seq == vd->sec_elmnt_seq)
			break;

	if (i < vcl->conf.sec_elmnt_count-1) {
		if (be32_to_cpu(vd->seqnum) <=
		    be32_to_cpu(vcl->other_bvds[i]->seqnum))
			return;
	} else {
		for (i = 0; i < vcl->conf.sec_elmnt_count-1; i++)
			if (vcl->other_bvds[i]->sec_elmnt_seq == DDF_UNUSED_BVD)
				break;
		if (i == vcl->conf.sec_elmnt_count-1) {
			pr_err("no space for sec level config %u, count is %u\n",
			       vd->sec_elmnt_seq, vcl->conf.sec_elmnt_count);
			return;
		}
	}
	memcpy(vcl->other_bvds[i], vd, len);
}

static int load_ddf_local(int fd, struct ddf_super *super,
			  char *devname, int keep)
{
	struct dl *dl;
	struct stat stb;
	char *conf;
	unsigned int i;
	unsigned int confsec;
	int vnum;
	unsigned int max_virt_disks =
		be16_to_cpu(super->active->max_vd_entries);
	unsigned long long dsize;

	/* First the local disk info */
	if (posix_memalign((void**)&dl, 512,
			   sizeof(*dl) +
			   (super->max_part) * sizeof(dl->vlist[0])) != 0) {
		pr_err("could not allocate disk info buffer\n");
		return 1;
	}

	load_section(fd, super, &dl->disk,
		     super->active->data_section_offset,
		     super->active->data_section_length,
		     0);
	dl->devname = devname ? xstrdup(devname) : NULL;

	fstat(fd, &stb);
	dl->major = major(stb.st_rdev);
	dl->minor = minor(stb.st_rdev);
	dl->next = super->dlist;
	dl->fd = keep ? fd : -1;

	dl->size = 0;
	if (get_dev_size(fd, devname, &dsize))
		dl->size = dsize >> 9;
	/* If the disks have different sizes, the LBAs will differ
	 * between phys disks.
	 * At this point here, the values in super->active must be valid
	 * for this phys disk. */
	dl->primary_lba = super->active->primary_lba;
	dl->secondary_lba = super->active->secondary_lba;
	dl->workspace_lba = super->active->workspace_lba;
	dl->spare = NULL;
	for (i = 0 ; i < super->max_part ; i++)
		dl->vlist[i] = NULL;
	super->dlist = dl;
	dl->pdnum = -1;
	for (i = 0; i < be16_to_cpu(super->active->max_pd_entries); i++)
		if (memcmp(super->phys->entries[i].guid,
			   dl->disk.guid, DDF_GUID_LEN) == 0)
			dl->pdnum = i;

	/* Now the config list. */
	/* 'conf' is an array of config entries, some of which are
	 * probably invalid.  Those which are good need to be copied into
	 * the conflist
	 */

	conf = load_section(fd, super, super->conf,
			    super->active->config_section_offset,
			    super->active->config_section_length,
			    0);
	super->conf = conf;
	vnum = 0;
	for (confsec = 0;
	     confsec < be32_to_cpu(super->active->config_section_length);
	     confsec += super->conf_rec_len) {
		struct vd_config *vd =
			(struct vd_config *)((char*)conf + confsec*512);
		struct vcl *vcl;

		if (be32_eq(vd->magic, DDF_SPARE_ASSIGN_MAGIC)) {
			if (dl->spare)
				continue;
			if (posix_memalign((void**)&dl->spare, 512,
					   super->conf_rec_len*512) != 0) {
				pr_err("could not allocate spare info buf\n");
				return 1;
			}

			memcpy(dl->spare, vd, super->conf_rec_len*512);
			continue;
		}
		if (!be32_eq(vd->magic, DDF_VD_CONF_MAGIC))
			/* Must be vendor-unique - I cannot handle those */
			continue;

		for (vcl = super->conflist; vcl; vcl = vcl->next) {
			if (memcmp(vcl->conf.guid,
				   vd->guid, DDF_GUID_LEN) == 0)
				break;
		}

		if (vcl) {
			dl->vlist[vnum++] = vcl;
			if (vcl->other_bvds != NULL &&
			    vcl->conf.sec_elmnt_seq != vd->sec_elmnt_seq) {
				add_other_bvd(vcl, vd, super->conf_rec_len*512);
				continue;
			}
			if (be32_to_cpu(vd->seqnum) <=
			    be32_to_cpu(vcl->conf.seqnum))
				continue;
		} else {
			if (posix_memalign((void**)&vcl, 512,
					   (super->conf_rec_len*512 +
					    offsetof(struct vcl, conf))) != 0) {
				pr_err("could not allocate vcl buf\n");
				return 1;
			}
			vcl->next = super->conflist;
			vcl->block_sizes = NULL; /* FIXME not for CONCAT */
			vcl->conf.sec_elmnt_count = vd->sec_elmnt_count;
			if (alloc_other_bvds(super, vcl) != 0) {
				pr_err("could not allocate other bvds\n");
				free(vcl);
				return 1;
			};
			super->conflist = vcl;
			dl->vlist[vnum++] = vcl;
		}
		memcpy(&vcl->conf, vd, super->conf_rec_len*512);
		for (i=0; i < max_virt_disks ; i++)
			if (memcmp(super->virt->entries[i].guid,
				   vcl->conf.guid, DDF_GUID_LEN)==0)
				break;
		if (i < max_virt_disks)
			vcl->vcnum = i;
	}

	return 0;
}

static int load_super_ddf(struct supertype *st, int fd,
			  char *devname)
{
	unsigned long long dsize;
	struct ddf_super *super;
	int rv;

	if (get_dev_size(fd, devname, &dsize) == 0)
		return 1;

	if (test_partition(fd))
		/* DDF is not allowed on partitions */
		return 1;

	/* 32M is a lower bound */
	if (dsize <= 32*1024*1024) {
		if (devname)
			pr_err("%s is too small for ddf: size is %llu sectors.\n",
			       devname, dsize>>9);
		return 1;
	}
	if (dsize & 511) {
		if (devname)
			pr_err("%s is an odd size for ddf: size is %llu bytes.\n",
			       devname, dsize);
		return 1;
	}

	free_super_ddf(st);

	if (posix_memalign((void**)&super, 512, sizeof(*super))!= 0) {
		pr_err("malloc of %zu failed.\n",
			sizeof(*super));
		return 1;
	}
	memset(super, 0, sizeof(*super));

	rv = load_ddf_headers(fd, super, devname);
	if (rv) {
		free(super);
		return rv;
	}

	/* Have valid headers and have chosen the best. Let's read in the rest*/

	rv = load_ddf_global(fd, super, devname);

	if (rv) {
		if (devname)
			pr_err("Failed to load all information sections on %s\n", devname);
		free(super);
		return rv;
	}

	rv = load_ddf_local(fd, super, devname, 0);

	if (rv) {
		if (devname)
			pr_err("Failed to load all information sections on %s\n", devname);
		free(super);
		return rv;
	}

	/* Should possibly check the sections .... */

	st->sb = super;
	if (st->ss == NULL) {
		st->ss = &super_ddf;
		st->minor_version = 0;
		st->max_devs = 512;
	}
	return 0;

}

static void free_super_ddf(struct supertype *st)
{
	struct ddf_super *ddf = st->sb;
	if (ddf == NULL)
		return;
	free(ddf->phys);
	free(ddf->virt);
	free(ddf->conf);
	while (ddf->conflist) {
		struct vcl *v = ddf->conflist;
		ddf->conflist = v->next;
		if (v->block_sizes)
			free(v->block_sizes);
		if (v->other_bvds)
			/*
			   v->other_bvds[0] points to beginning of buffer,
			   see alloc_other_bvds()
			*/
			free(v->other_bvds[0]);
		free(v);
	}
	while (ddf->dlist) {
		struct dl *d = ddf->dlist;
		ddf->dlist = d->next;
		if (d->fd >= 0)
			close(d->fd);
		if (d->spare)
			free(d->spare);
		free(d);
	}
	while (ddf->add_list) {
		struct dl *d = ddf->add_list;
		ddf->add_list = d->next;
		if (d->fd >= 0)
			close(d->fd);
		if (d->spare)
			free(d->spare);
		free(d);
	}
	free(ddf);
	st->sb = NULL;
}

static struct supertype *match_metadata_desc_ddf(char *arg)
{
	/* 'ddf' only supports containers */
	struct supertype *st;
	if (strcmp(arg, "ddf") != 0 &&
	    strcmp(arg, "default") != 0
		)
		return NULL;

	st = xcalloc(1, sizeof(*st));
	st->ss = &super_ddf;
	st->max_devs = 512;
	st->minor_version = 0;
	st->sb = NULL;
	return st;
}

static mapping_t ddf_state[] = {
	{ "Optimal", 0},
	{ "Degraded", 1},
	{ "Deleted", 2},
	{ "Missing", 3},
	{ "Failed", 4},
	{ "Partially Optimal", 5},
	{ "-reserved-", 6},
	{ "-reserved-", 7},
	{ NULL, 0}
};

static mapping_t ddf_init_state[] = {
	{ "Not Initialised", 0},
	{ "QuickInit in Progress", 1},
	{ "Fully Initialised", 2},
	{ "*UNKNOWN*", 3},
	{ NULL, 0}
};
static mapping_t ddf_access[] = {
	{ "Read/Write", 0},
	{ "Reserved", 1},
	{ "Read Only", 2},
	{ "Blocked (no access)", 3},
	{ NULL ,0}
};

static mapping_t ddf_level[] = {
	{ "RAID0", DDF_RAID0},
	{ "RAID1", DDF_RAID1},
	{ "RAID3", DDF_RAID3},
	{ "RAID4", DDF_RAID4},
	{ "RAID5", DDF_RAID5},
	{ "RAID1E",DDF_RAID1E},
	{ "JBOD",  DDF_JBOD},
	{ "CONCAT",DDF_CONCAT},
	{ "RAID5E",DDF_RAID5E},
	{ "RAID5EE",DDF_RAID5EE},
	{ "RAID6", DDF_RAID6},
	{ NULL, 0}
};
static mapping_t ddf_sec_level[] = {
	{ "Striped", DDF_2STRIPED},
	{ "Mirrored", DDF_2MIRRORED},
	{ "Concat", DDF_2CONCAT},
	{ "Spanned", DDF_2SPANNED},
	{ NULL, 0}
};

static int all_ff(const char *guid)
{
	int i;
	for (i = 0; i < DDF_GUID_LEN; i++)
		if (guid[i] != (char)0xff)
			return 0;
	return 1;
}

static const char *guid_str(const char *guid)
{
	static char buf[DDF_GUID_LEN*2+1];
	int i;
	char *p = buf;
	for (i = 0; i < DDF_GUID_LEN; i++) {
		unsigned char c = guid[i];
		if (c >= 32 && c < 127)
			p += sprintf(p, "%c", c);
		else
			p += sprintf(p, "%02x", c);
	}
	*p = '\0';
	return (const char *) buf;
}

static void print_guid(char *guid, int tstamp)
{
	/* A GUIDs are part (or all) ASCII and part binary.
	 * They tend to be space padded.
	 * We print the GUID in HEX, then in parentheses add
	 * any initial ASCII sequence, and a possible
	 * time stamp from bytes 16-19
	 */
	int l = DDF_GUID_LEN;
	int i;

	for (i=0 ; i<DDF_GUID_LEN ; i++) {
		if ((i&3)==0 && i != 0) printf(":");
		printf("%02X", guid[i]&255);
	}

	printf("\n                  (");
	while (l && guid[l-1] == ' ')
		l--;
	for (i=0 ; i<l ; i++) {
		if (guid[i] >= 0x20 && guid[i] < 0x7f)
			fputc(guid[i], stdout);
		else
			break;
	}
	if (tstamp) {
		time_t then = __be32_to_cpu(*(__u32*)(guid+16)) + DECADE;
		char tbuf[100];
		struct tm *tm;
		tm = localtime(&then);
		strftime(tbuf, 100, " %D %T",tm);
		fputs(tbuf, stdout);
	}
	printf(")");
}

static void examine_vd(int n, struct ddf_super *sb, char *guid)
{
	int crl = sb->conf_rec_len;
	struct vcl *vcl;

	for (vcl = sb->conflist ; vcl ; vcl = vcl->next) {
		unsigned int i;
		struct vd_config *vc = &vcl->conf;

		if (!be32_eq(calc_crc(vc, crl*512), vc->crc))
			continue;
		if (memcmp(vc->guid, guid, DDF_GUID_LEN) != 0)
			continue;

		/* Ok, we know about this VD, let's give more details */
		printf(" Raid Devices[%d] : %d (", n,
		       be16_to_cpu(vc->prim_elmnt_count));
		for (i = 0; i < be16_to_cpu(vc->prim_elmnt_count); i++) {
			int j;
			int cnt = be16_to_cpu(sb->phys->max_pdes);
			for (j=0; j<cnt; j++)
				if (be32_eq(vc->phys_refnum[i],
					    sb->phys->entries[j].refnum))
					break;
			if (i) printf(" ");
			if (j < cnt)
				printf("%d", j);
			else
				printf("--");
			printf("@%lluK", (unsigned long long) be64_to_cpu(LBA_OFFSET(sb, vc)[i])/2);
		}
		printf(")\n");
		if (vc->chunk_shift != 255)
			printf("   Chunk Size[%d] : %d sectors\n", n,
			       1 << vc->chunk_shift);
		printf("   Raid Level[%d] : %s\n", n,
		       map_num(ddf_level, vc->prl)?:"-unknown-");
		if (vc->sec_elmnt_count != 1) {
			printf("  Secondary Position[%d] : %d of %d\n", n,
			       vc->sec_elmnt_seq, vc->sec_elmnt_count);
			printf("  Secondary Level[%d] : %s\n", n,
			       map_num(ddf_sec_level, vc->srl) ?: "-unknown-");
		}
		printf("  Device Size[%d] : %llu\n", n,
		       be64_to_cpu(vc->blocks)/2);
		printf("   Array Size[%d] : %llu\n", n,
		       be64_to_cpu(vc->array_blocks)/2);
	}
}

static void examine_vds(struct ddf_super *sb)
{
	int cnt = be16_to_cpu(sb->virt->populated_vdes);
	unsigned int i;
	printf("  Virtual Disks : %d\n", cnt);

	for (i = 0; i < be16_to_cpu(sb->virt->max_vdes); i++) {
		struct virtual_entry *ve = &sb->virt->entries[i];
		if (all_ff(ve->guid))
			continue;
		printf("\n");
		printf("      VD GUID[%d] : ", i); print_guid(ve->guid, 1);
		printf("\n");
		printf("         unit[%d] : %d\n", i, be16_to_cpu(ve->unit));
		printf("        state[%d] : %s, %s%s\n", i,
		       map_num(ddf_state, ve->state & 7),
		       (ve->state & DDF_state_morphing) ? "Morphing, ": "",
		       (ve->state & DDF_state_inconsistent)? "Not Consistent" : "Consistent");
		printf("   init state[%d] : %s\n", i,
		       map_num(ddf_init_state, ve->init_state&DDF_initstate_mask));
		printf("       access[%d] : %s\n", i,
		       map_num(ddf_access, (ve->init_state & DDF_access_mask) >> 6));
		printf("         Name[%d] : %.16s\n", i, ve->name);
		examine_vd(i, sb, ve->guid);
	}
	if (cnt) printf("\n");
}

static void examine_pds(struct ddf_super *sb)
{
	int cnt = be16_to_cpu(sb->phys->max_pdes);
	int i;
	struct dl *dl;
	int unlisted = 0;
	printf(" Physical Disks : %d\n", cnt);
	printf("      Number    RefNo      Size       Device      Type/State\n");

	for (dl = sb->dlist; dl; dl = dl->next)
		dl->displayed = 0;

	for (i=0 ; i<cnt ; i++) {
		struct phys_disk_entry *pd = &sb->phys->entries[i];
		int type = be16_to_cpu(pd->type);
		int state = be16_to_cpu(pd->state);

		if (be32_to_cpu(pd->refnum) == 0xffffffff)
			/* Not in use */
			continue;
		//printf("      PD GUID[%d] : ", i); print_guid(pd->guid, 0);
		//printf("\n");
		printf("       %3d    %08x  ", i,
		       be32_to_cpu(pd->refnum));
		printf("%8lluK ",
		       be64_to_cpu(pd->config_size)>>1);
		for (dl = sb->dlist; dl ; dl = dl->next) {
			if (be32_eq(dl->disk.refnum, pd->refnum)) {
				char *dv = map_dev(dl->major, dl->minor, 0);
				if (dv) {
					printf("%-15s", dv);
					break;
				}
			}
		}
		if (!dl)
			printf("%15s","");
		else
			dl->displayed = 1;
		printf(" %s%s%s%s%s",
		       (type&2) ? "active":"",
		       (type&4) ? "Global-Spare":"",
		       (type&8) ? "spare" : "",
		       (type&16)? ", foreign" : "",
		       (type&32)? "pass-through" : "");
		if (state & DDF_Failed)
			/* This over-rides these three */
			state &= ~(DDF_Online|DDF_Rebuilding|DDF_Transition);
		printf("/%s%s%s%s%s%s%s",
		       (state&1)? "Online": "Offline",
		       (state&2)? ", Failed": "",
		       (state&4)? ", Rebuilding": "",
		       (state&8)? ", in-transition": "",
		       (state&16)? ", SMART-errors": "",
		       (state&32)? ", Unrecovered-Read-Errors": "",
		       (state&64)? ", Missing" : "");
		printf("\n");
	}
	for (dl = sb->dlist; dl; dl = dl->next) {
		char *dv;
		if (dl->displayed)
			continue;
		if (!unlisted)
			printf(" Physical disks not in metadata!:\n");
		unlisted = 1;
		dv = map_dev(dl->major, dl->minor, 0);
		printf("   %08x %s\n", be32_to_cpu(dl->disk.refnum),
		       dv ? dv : "-unknown-");
	}
	if (unlisted)
		printf("\n");
}

static void examine_super_ddf(struct supertype *st, char *homehost)
{
	struct ddf_super *sb = st->sb;

	printf("          Magic : %08x\n", be32_to_cpu(sb->anchor.magic));
	printf("        Version : %.8s\n", sb->anchor.revision);
	printf("Controller GUID : "); print_guid(sb->controller.guid, 0);
	printf("\n");
	printf(" Container GUID : "); print_guid(sb->anchor.guid, 1);
	printf("\n");
	printf("            Seq : %08x\n", be32_to_cpu(sb->active->seq));
	printf("  Redundant hdr : %s\n", (be32_eq(sb->secondary.magic,
						 DDF_HEADER_MAGIC)
					  ?"yes" : "no"));
	examine_vds(sb);
	examine_pds(sb);
}

static unsigned int get_vd_num_of_subarray(struct supertype *st)
{
	/*
	 * Figure out the VD number for this supertype.
	 * Returns DDF_CONTAINER for the container itself,
	 * and DDF_NOTFOUND on error.
	 */
	struct ddf_super *ddf = st->sb;
	struct mdinfo *sra;
	char *sub, *end;
	unsigned int vcnum;

	if (*st->container_devnm == '\0')
		return DDF_CONTAINER;

	sra = sysfs_read(-1, st->devnm, GET_VERSION);
	if (!sra || sra->array.major_version != -1 ||
	    sra->array.minor_version != -2 ||
	    !is_subarray(sra->text_version))
		return DDF_NOTFOUND;

	sub = strchr(sra->text_version + 1, '/');
	if (sub != NULL)
		vcnum = strtoul(sub + 1, &end, 10);
	if (sub == NULL || *sub == '\0' || *end != '\0' ||
	    vcnum >= be16_to_cpu(ddf->active->max_vd_entries))
		return DDF_NOTFOUND;

	return vcnum;
}

static void brief_examine_super_ddf(struct supertype *st, int verbose)
{
	/* We just write a generic DDF ARRAY entry
	 */
	struct mdinfo info;
	char nbuf[64];
	getinfo_super_ddf(st, &info, NULL);
	fname_from_uuid(st, &info, nbuf, ':');

	printf("ARRAY metadata=ddf UUID=%s\n", nbuf + 5);
}

static void brief_examine_subarrays_ddf(struct supertype *st, int verbose)
{
	/* We write a DDF ARRAY member entry for each vd, identifying container
	 * by uuid and member by unit number and uuid.
	 */
	struct ddf_super *ddf = st->sb;
	struct mdinfo info;
	unsigned int i;
	char nbuf[64];
	getinfo_super_ddf(st, &info, NULL);
	fname_from_uuid(st, &info, nbuf, ':');

	for (i = 0; i < be16_to_cpu(ddf->virt->max_vdes); i++) {
		struct virtual_entry *ve = &ddf->virt->entries[i];
		struct vcl vcl;
		char nbuf1[64];
		char namebuf[17];
		if (all_ff(ve->guid))
			continue;
		memcpy(vcl.conf.guid, ve->guid, DDF_GUID_LEN);
		ddf->currentconf =&vcl;
		vcl.vcnum = i;
		uuid_from_super_ddf(st, info.uuid);
		fname_from_uuid(st, &info, nbuf1, ':');
		_ddf_array_name(namebuf, ddf, i);
		printf("ARRAY%s%s container=%s member=%d UUID=%s\n",
		       namebuf[0] == '\0' ? "" : " /dev/md/", namebuf,
		       nbuf+5, i, nbuf1+5);
	}
}

static void export_examine_super_ddf(struct supertype *st)
{
	struct mdinfo info;
	char nbuf[64];
	getinfo_super_ddf(st, &info, NULL);
	fname_from_uuid(st, &info, nbuf, ':');
	printf("MD_METADATA=ddf\n");
	printf("MD_LEVEL=container\n");
	printf("MD_UUID=%s\n", nbuf+5);
	printf("MD_DEVICES=%u\n",
		be16_to_cpu(((struct ddf_super *)st->sb)->phys->used_pdes));
}

static int copy_metadata_ddf(struct supertype *st, int from, int to)
{
	void *buf;
	unsigned long long dsize, offset;
	int bytes;
	struct ddf_header *ddf;
	int written = 0;

	/* The meta consists of an anchor, a primary, and a secondary.
	 * This all lives at the end of the device.
	 * So it is easiest to find the earliest of primary and
	 * secondary, and copy everything from there.
	 *
	 * Anchor is 512 from end.  It contains primary_lba and secondary_lba
	 * we choose one of those
	 */

	if (posix_memalign(&buf, 4096, 4096) != 0)
		return 1;

	if (!get_dev_size(from, NULL, &dsize))
		goto err;

	if (lseek64(from, dsize-512, 0) < 0)
		goto err;
	if (read(from, buf, 512) != 512)
		goto err;
	ddf = buf;
	if (!be32_eq(ddf->magic, DDF_HEADER_MAGIC) ||
	    !be32_eq(calc_crc(ddf, 512), ddf->crc) ||
	    (memcmp(ddf->revision, DDF_REVISION_0, 8) != 0 &&
	     memcmp(ddf->revision, DDF_REVISION_2, 8) != 0))
		goto err;

	offset = dsize - 512;
	if ((be64_to_cpu(ddf->primary_lba) << 9) < offset)
		offset = be64_to_cpu(ddf->primary_lba) << 9;
	if ((be64_to_cpu(ddf->secondary_lba) << 9) < offset)
		offset = be64_to_cpu(ddf->secondary_lba) << 9;

	bytes = dsize - offset;

	if (lseek64(from, offset, 0) < 0 ||
	    lseek64(to, offset, 0) < 0)
		goto err;
	while (written < bytes) {
		int n = bytes - written;
		if (n > 4096)
			n = 4096;
		if (read(from, buf, n) != n)
			goto err;
		if (write(to, buf, n) != n)
			goto err;
		written += n;
	}
	free(buf);
	return 0;
err:
	free(buf);
	return 1;
}

static void detail_super_ddf(struct supertype *st, char *homehost)
{
	struct ddf_super *sb = st->sb;
	int cnt = be16_to_cpu(sb->virt->populated_vdes);

	printf("    Container GUID : "); print_guid(sb->anchor.guid, 1);
	printf("\n");
	printf("               Seq : %08x\n", be32_to_cpu(sb->active->seq));
	printf("     Virtual Disks : %d\n", cnt);
	printf("\n");
}

static const char *vendors_with_variable_volume_UUID[] = {
	"LSI      ",
};

static int volume_id_is_reliable(const struct ddf_super *ddf)
{
	int n = ARRAY_SIZE(vendors_with_variable_volume_UUID);
	int i;
	for (i = 0; i < n; i++)
		if (!memcmp(ddf->controller.guid,
			vendors_with_variable_volume_UUID[i], 8))
		return 0;
	return 1;
}

static void uuid_of_ddf_subarray(const struct ddf_super *ddf,
				 unsigned int vcnum, int uuid[4])
{
	char buf[DDF_GUID_LEN+18], sha[20], *p;
	struct sha1_ctx ctx;
	if (volume_id_is_reliable(ddf)) {
		uuid_from_ddf_guid(ddf->virt->entries[vcnum].guid, uuid);
		return;
	}
	/*
	 * Some fake RAID BIOSes (in particular, LSI ones) change the
	 * VD GUID at every boot. These GUIDs are not suitable for
	 * identifying an array. Luckily the header GUID appears to
	 * remain constant.
	 * We construct a pseudo-UUID from the header GUID and those
	 * properties of the subarray that we expect to remain constant.
	 */
	memset(buf, 0, sizeof(buf));
	p = buf;
	memcpy(p, ddf->anchor.guid, DDF_GUID_LEN);
	p += DDF_GUID_LEN;
	memcpy(p, ddf->virt->entries[vcnum].name, 16);
	p += 16;
	*((__u16 *) p) = vcnum;
	sha1_init_ctx(&ctx);
	sha1_process_bytes(buf, sizeof(buf), &ctx);
	sha1_finish_ctx(&ctx, sha);
	memcpy(uuid, sha, 4*4);
}

static void brief_detail_super_ddf(struct supertype *st)
{
	struct mdinfo info;
	char nbuf[64];
	struct ddf_super *ddf = st->sb;
	unsigned int vcnum = get_vd_num_of_subarray(st);
	if (vcnum == DDF_CONTAINER)
		uuid_from_super_ddf(st, info.uuid);
	else if (vcnum == DDF_NOTFOUND)
		return;
	else
		uuid_of_ddf_subarray(ddf, vcnum, info.uuid);
	fname_from_uuid(st, &info, nbuf,':');
	printf(" UUID=%s", nbuf + 5);
}

static int match_home_ddf(struct supertype *st, char *homehost)
{
	/* It matches 'this' host if the controller is a
	 * Linux-MD controller with vendor_data matching
	 * the hostname.  It would be nice if we could
	 * test against controller found in /sys or somewhere...
	 */
	struct ddf_super *ddf = st->sb;
	unsigned int len;

	if (!homehost)
		return 0;
	len = strlen(homehost);

	return (memcmp(ddf->controller.guid, T10, 8) == 0 &&
		len < sizeof(ddf->controller.vendor_data) &&
		memcmp(ddf->controller.vendor_data, homehost,len) == 0 &&
		ddf->controller.vendor_data[len] == 0);
}

static int find_index_in_bvd(const struct ddf_super *ddf,
			     const struct vd_config *conf, unsigned int n,
			     unsigned int *n_bvd)
{
	/*
	 * Find the index of the n-th valid physical disk in this BVD.
	 * Unused entries can be sprinkled in with the used entries,
	 * but don't count.
	 */
	unsigned int i, j;
	for (i = 0, j = 0;
	     i < ddf->mppe && j < be16_to_cpu(conf->prim_elmnt_count);
	     i++) {
		if (be32_to_cpu(conf->phys_refnum[i]) != 0xffffffff) {
			if (n == j) {
				*n_bvd = i;
				return 1;
			}
			j++;
		}
	}
	dprintf("couldn't find BVD member %u (total %u)\n",
		n, be16_to_cpu(conf->prim_elmnt_count));
	return 0;
}

/* Given a member array instance number, and a raid disk within that instance,
 * find the vd_config structure.  The offset of the given disk in the phys_refnum
 * table is returned in n_bvd.
 * For two-level members with a secondary raid level the vd_config for
 * the appropriate BVD is returned.
 * The return value is always &vlc->conf, where vlc is returned in last pointer.
 */
static struct vd_config *find_vdcr(struct ddf_super *ddf, unsigned int inst,
				   unsigned int n,
				   unsigned int *n_bvd, struct vcl **vcl)
{
	struct vcl *v;

	for (v = ddf->conflist; v; v = v->next) {
		unsigned int nsec, ibvd = 0;
		struct vd_config *conf;
		if (inst != v->vcnum)
			continue;
		conf = &v->conf;
		if (conf->sec_elmnt_count == 1) {
			if (find_index_in_bvd(ddf, conf, n, n_bvd)) {
				*vcl = v;
				return conf;
			} else
				goto bad;
		}
		if (v->other_bvds == NULL) {
			pr_err("BUG: other_bvds is NULL, nsec=%u\n",
			       conf->sec_elmnt_count);
			goto bad;
		}
		nsec = n / be16_to_cpu(conf->prim_elmnt_count);
		if (conf->sec_elmnt_seq != nsec) {
			for (ibvd = 1; ibvd < conf->sec_elmnt_count; ibvd++) {
				if (v->other_bvds[ibvd-1]->sec_elmnt_seq ==
				    nsec)
					break;
			}
			if (ibvd == conf->sec_elmnt_count)
				goto bad;
			conf = v->other_bvds[ibvd-1];
		}
		if (!find_index_in_bvd(ddf, conf,
				       n - nsec*conf->sec_elmnt_count, n_bvd))
			goto bad;
		dprintf("found disk %u as member %u in bvd %d of array %u\n",
			n, *n_bvd, ibvd, inst);
		*vcl = v;
		return conf;
	}
bad:
	pr_err("Could't find disk %d in array %u\n", n, inst);
	return NULL;
}

static int find_phys(const struct ddf_super *ddf, be32 phys_refnum)
{
	/* Find the entry in phys_disk which has the given refnum
	 * and return it's index
	 */
	unsigned int i;
	for (i = 0; i < be16_to_cpu(ddf->phys->max_pdes); i++)
		if (be32_eq(ddf->phys->entries[i].refnum, phys_refnum))
			return i;
	return -1;
}

static void uuid_from_ddf_guid(const char *guid, int uuid[4])
{
	char buf[20];
	struct sha1_ctx ctx;
	sha1_init_ctx(&ctx);
	sha1_process_bytes(guid, DDF_GUID_LEN, &ctx);
	sha1_finish_ctx(&ctx, buf);
	memcpy(uuid, buf, 4*4);
}

static void uuid_from_super_ddf(struct supertype *st, int uuid[4])
{
	/* The uuid returned here is used for:
	 *  uuid to put into bitmap file (Create, Grow)
	 *  uuid for backup header when saving critical section (Grow)
	 *  comparing uuids when re-adding a device into an array
	 *    In these cases the uuid required is that of the data-array,
	 *    not the device-set.
	 *  uuid to recognise same set when adding a missing device back
	 *    to an array.   This is a uuid for the device-set.
	 *
	 * For each of these we can make do with a truncated
	 * or hashed uuid rather than the original, as long as
	 * everyone agrees.
	 * In the case of SVD we assume the BVD is of interest,
	 * though that might be the case if a bitmap were made for
	 * a mirrored SVD - worry about that later.
	 * So we need to find the VD configuration record for the
	 * relevant BVD and extract the GUID and Secondary_Element_Seq.
	 * The first 16 bytes of the sha1 of these is used.
	 */
	struct ddf_super *ddf = st->sb;
	struct vcl *vcl = ddf->currentconf;

	if (vcl)
		uuid_of_ddf_subarray(ddf, vcl->vcnum, uuid);
	else
		uuid_from_ddf_guid(ddf->anchor.guid, uuid);
}

static void getinfo_super_ddf(struct supertype *st, struct mdinfo *info, char *map)
{
	struct ddf_super *ddf = st->sb;
	int map_disks = info->array.raid_disks;
	__u32 *cptr;

	if (ddf->currentconf) {
		getinfo_super_ddf_bvd(st, info, map);
		return;
	}
	memset(info, 0, sizeof(*info));

	info->array.raid_disks    = be16_to_cpu(ddf->phys->used_pdes);
	info->array.level	  = LEVEL_CONTAINER;
	info->array.layout	  = 0;
	info->array.md_minor	  = -1;
	cptr = (__u32 *)(ddf->anchor.guid + 16);
	info->array.ctime	  = DECADE + __be32_to_cpu(*cptr);

	info->array.chunk_size	  = 0;
	info->container_enough	  = 1;

	info->disk.major	  = 0;
	info->disk.minor	  = 0;
	if (ddf->dlist) {
		struct phys_disk_entry *pde = NULL;
		info->disk.number = be32_to_cpu(ddf->dlist->disk.refnum);
		info->disk.raid_disk = find_phys(ddf, ddf->dlist->disk.refnum);

		info->data_offset = be64_to_cpu(ddf->phys->
						  entries[info->disk.raid_disk].
						  config_size);
		info->component_size = ddf->dlist->size - info->data_offset;
		if (info->disk.raid_disk >= 0)
			pde = ddf->phys->entries + info->disk.raid_disk;
		if (pde &&
		    !(be16_to_cpu(pde->state) & DDF_Failed) &&
		    !(be16_to_cpu(pde->state) & DDF_Missing))
			info->disk.state = (1 << MD_DISK_SYNC) | (1 << MD_DISK_ACTIVE);
		else
			info->disk.state = 1 << MD_DISK_FAULTY;

	} else {
		/* There should always be a dlist, but just in case...*/
		info->disk.number = -1;
		info->disk.raid_disk = -1;
		info->disk.state = (1 << MD_DISK_SYNC) | (1 << MD_DISK_ACTIVE);
	}
	info->events = be32_to_cpu(ddf->active->seq);
	info->array.utime = DECADE + be32_to_cpu(ddf->active->timestamp);

	info->recovery_start = MaxSector;
	info->reshape_active = 0;
	info->recovery_blocked = 0;
	info->name[0] = 0;

	info->array.major_version = -1;
	info->array.minor_version = -2;
	strcpy(info->text_version, "ddf");
	info->safe_mode_delay = 0;

	uuid_from_super_ddf(st, info->uuid);

	if (map) {
		int i, e = 0;
		int max = be16_to_cpu(ddf->phys->max_pdes);
		for (i = e = 0 ; i < map_disks ; i++, e++) {
			while (e < max &&
			       be32_to_cpu(ddf->phys->entries[e].refnum) == 0xffffffff)
				e++;
			if (i < info->array.raid_disks && e < max &&
			    !(be16_to_cpu(ddf->phys->entries[e].state) &
			      DDF_Failed))
				map[i] = 1;
			else
				map[i] = 0;
		}
	}
}

/* size of name must be at least 17 bytes! */
static void _ddf_array_name(char *name, const struct ddf_super *ddf, int i)
{
	int j;
	memcpy(name, ddf->virt->entries[i].name, 16);
	name[16] = 0;
	for(j = 0; j < 16; j++)
		if (name[j] == ' ')
			name[j] = 0;
}

static void getinfo_super_ddf_bvd(struct supertype *st, struct mdinfo *info, char *map)
{
	struct ddf_super *ddf = st->sb;
	struct vcl *vc = ddf->currentconf;
	int cd = ddf->currentdev;
	int n_prim;
	int j;
	struct dl *dl = NULL;
	int map_disks = info->array.raid_disks;
	__u32 *cptr;
	struct vd_config *conf;

	memset(info, 0, sizeof(*info));
	if (layout_ddf2md(&vc->conf, &info->array) == -1)
		return;
	info->array.md_minor	  = -1;
	cptr = (__u32 *)(vc->conf.guid + 16);
	info->array.ctime	  = DECADE + __be32_to_cpu(*cptr);
	info->array.utime	  = DECADE + be32_to_cpu(vc->conf.timestamp);
	info->array.chunk_size	  = 512 << vc->conf.chunk_shift;
	info->custom_array_size	  = be64_to_cpu(vc->conf.array_blocks);

	conf = &vc->conf;
	n_prim = be16_to_cpu(conf->prim_elmnt_count);
	if (conf->sec_elmnt_count > 1 && cd >= n_prim) {
		int ibvd = cd / n_prim - 1;
		cd %= n_prim;
		conf = vc->other_bvds[ibvd];
	}

	if (cd >= 0 && (unsigned)cd < ddf->mppe) {
		info->data_offset =
			be64_to_cpu(LBA_OFFSET(ddf, conf)[cd]);
		if (vc->block_sizes)
			info->component_size = vc->block_sizes[cd];
		else
			info->component_size = be64_to_cpu(conf->blocks);

		for (dl = ddf->dlist; dl ; dl = dl->next)
			if (be32_eq(dl->disk.refnum, conf->phys_refnum[cd]))
				break;
	}

	info->disk.major = 0;
	info->disk.minor = 0;
	info->disk.state = 0;
	if (dl && dl->pdnum >= 0) {
		info->disk.major = dl->major;
		info->disk.minor = dl->minor;
		info->disk.raid_disk = cd + conf->sec_elmnt_seq
			* be16_to_cpu(conf->prim_elmnt_count);
		info->disk.number = dl->pdnum;
		info->disk.state = 0;
		if (info->disk.number >= 0 &&
		    (be16_to_cpu(ddf->phys->entries[info->disk.number].state) & DDF_Online) &&
		    !(be16_to_cpu(ddf->phys->entries[info->disk.number].state) & DDF_Failed))
			info->disk.state = (1<<MD_DISK_SYNC)|(1<<MD_DISK_ACTIVE);
		info->events = be32_to_cpu(ddf->active->seq);
	}

	info->container_member = ddf->currentconf->vcnum;

	info->recovery_start = MaxSector;
	info->resync_start = 0;
	info->reshape_active = 0;
	info->recovery_blocked = 0;
	if (!(ddf->virt->entries[info->container_member].state &
	      DDF_state_inconsistent) &&
	    (ddf->virt->entries[info->container_member].init_state &
	     DDF_initstate_mask) == DDF_init_full)
		info->resync_start = MaxSector;

	uuid_from_super_ddf(st, info->uuid);

	info->array.major_version = -1;
	info->array.minor_version = -2;
	sprintf(info->text_version, "/%s/%d",
		st->container_devnm,
		info->container_member);
	info->safe_mode_delay = DDF_SAFE_MODE_DELAY;

	_ddf_array_name(info->name, ddf, info->container_member);

	if (map)
		for (j = 0; j < map_disks; j++) {
			map[j] = 0;
			if (j < info->array.raid_disks) {
				int i = find_phys(ddf, vc->conf.phys_refnum[j]);
				if (i >= 0 &&
				    (be16_to_cpu(ddf->phys->entries[i].state)
				     & DDF_Online) &&
				    !(be16_to_cpu(ddf->phys->entries[i].state)
				      & DDF_Failed))
					map[i] = 1;
			}
		}
}

static int update_super_ddf(struct supertype *st, struct mdinfo *info,
			    char *update,
			    char *devname, int verbose,
			    int uuid_set, char *homehost)
{
	/* For 'assemble' and 'force' we need to return non-zero if any
	 * change was made.  For others, the return value is ignored.
	 * Update options are:
	 *  force-one : This device looks a bit old but needs to be included,
	 *        update age info appropriately.
	 *  assemble: clear any 'faulty' flag to allow this device to
	 *		be assembled.
	 *  force-array: Array is degraded but being forced, mark it clean
	 *	   if that will be needed to assemble it.
	 *
	 *  newdev:  not used ????
	 *  grow:  Array has gained a new device - this is currently for
	 *		linear only
	 *  resync: mark as dirty so a resync will happen.
	 *  uuid:  Change the uuid of the array to match what is given
	 *  homehost:  update the recorded homehost
	 *  name:  update the name - preserving the homehost
	 *  _reshape_progress: record new reshape_progress position.
	 *
	 * Following are not relevant for this version:
	 *  sparc2.2 : update from old dodgey metadata
	 *  super-minor: change the preferred_minor number
	 *  summaries:  update redundant counters.
	 */
	int rv = 0;
//	struct ddf_super *ddf = st->sb;
//	struct vd_config *vd = find_vdcr(ddf, info->container_member);
//	struct virtual_entry *ve = find_ve(ddf);

	/* we don't need to handle "force-*" or "assemble" as
	 * there is no need to 'trick' the kernel.  When the metadata is
	 * first updated to activate the array, all the implied modifications
	 * will just happen.
	 */

	if (strcmp(update, "grow") == 0) {
		/* FIXME */
	} else if (strcmp(update, "resync") == 0) {
//		info->resync_checkpoint = 0;
	} else if (strcmp(update, "homehost") == 0) {
		/* homehost is stored in controller->vendor_data,
		 * or it is when we are the vendor
		 */
//		if (info->vendor_is_local)
//			strcpy(ddf->controller.vendor_data, homehost);
		rv = -1;
	} else if (strcmp(update, "name") == 0) {
		/* name is stored in virtual_entry->name */
//		memset(ve->name, ' ', 16);
//		strncpy(ve->name, info->name, 16);
		rv = -1;
	} else if (strcmp(update, "_reshape_progress") == 0) {
		/* We don't support reshape yet */
	} else if (strcmp(update, "assemble") == 0 ) {
		/* Do nothing, just succeed */
		rv = 0;
	} else
		rv = -1;

//	update_all_csum(ddf);

	return rv;
}

static void make_header_guid(char *guid)
{
	be32 stamp;
	/* Create a DDF Header of Virtual Disk GUID */

	/* 24 bytes of fiction required.
	 * first 8 are a 'vendor-id'  - "Linux-MD"
	 * next 8 are controller type.. how about 0X DEAD BEEF 0000 0000
	 * Remaining 8 random number plus timestamp
	 */
	memcpy(guid, T10, sizeof(T10));
	stamp = cpu_to_be32(0xdeadbeef);
	memcpy(guid+8, &stamp, 4);
	stamp = cpu_to_be32(0);
	memcpy(guid+12, &stamp, 4);
	stamp = cpu_to_be32(time(0) - DECADE);
	memcpy(guid+16, &stamp, 4);
	stamp._v32 = random32();
	memcpy(guid+20, &stamp, 4);
}

static unsigned int find_unused_vde(const struct ddf_super *ddf)
{
	unsigned int i;
	for (i = 0; i < be16_to_cpu(ddf->virt->max_vdes); i++) {
		if (all_ff(ddf->virt->entries[i].guid))
			return i;
	}
	return DDF_NOTFOUND;
}

static unsigned int find_vde_by_name(const struct ddf_super *ddf,
				     const char *name)
{
	unsigned int i;
	if (name == NULL)
		return DDF_NOTFOUND;
	for (i = 0; i < be16_to_cpu(ddf->virt->max_vdes); i++) {
		if (all_ff(ddf->virt->entries[i].guid))
			continue;
		if (!strncmp(name, ddf->virt->entries[i].name,
			     sizeof(ddf->virt->entries[i].name)))
			return i;
	}
	return DDF_NOTFOUND;
}

static unsigned int find_vde_by_guid(const struct ddf_super *ddf,
				     const char *guid)
{
	unsigned int i;
	if (guid == NULL || all_ff(guid))
		return DDF_NOTFOUND;
	for (i = 0; i < be16_to_cpu(ddf->virt->max_vdes); i++)
		if (!memcmp(ddf->virt->entries[i].guid, guid, DDF_GUID_LEN))
			return i;
	return DDF_NOTFOUND;
}

static int init_super_ddf(struct supertype *st,
			  mdu_array_info_t *info,
			  struct shape *s, char *name, char *homehost,
			  int *uuid, unsigned long long data_offset)
{
	/* This is primarily called by Create when creating a new array.
	 * We will then get add_to_super called for each component, and then
	 * write_init_super called to write it out to each device.
	 * For DDF, Create can create on fresh devices or on a pre-existing
	 * array.
	 * To create on a pre-existing array a different method will be called.
	 * This one is just for fresh drives.
	 *
	 * We need to create the entire 'ddf' structure which includes:
	 *  DDF headers - these are easy.
	 *  Controller data - a Sector describing this controller .. not that
	 *		      this is a controller exactly.
	 *  Physical Disk Record - one entry per device, so
	 *			   leave plenty of space.
	 *  Virtual Disk Records - again, just leave plenty of space.
	 *			   This just lists VDs, doesn't give details.
	 *  Config records - describe the VDs that use this disk
	 *  DiskData  - describes 'this' device.
	 *  BadBlockManagement - empty
	 *  Diag Space - empty
	 *  Vendor Logs - Could we put bitmaps here?
	 *
	 */
	struct ddf_super *ddf;
	char hostname[17];
	int hostlen;
	int max_phys_disks, max_virt_disks;
	unsigned long long sector;
	int clen;
	int i;
	int pdsize, vdsize;
	struct phys_disk *pd;
	struct virtual_disk *vd;

	if (st->sb)
		return init_super_ddf_bvd(st, info, s->size, name, homehost, uuid,
					  data_offset);

	if (posix_memalign((void**)&ddf, 512, sizeof(*ddf)) != 0) {
		pr_err("could not allocate superblock\n");
		return 0;
	}
	memset(ddf, 0, sizeof(*ddf));
	st->sb = ddf;

	if (info == NULL) {
		/* zeroing superblock */
		return 0;
	}

	/* At least 32MB *must* be reserved for the ddf.  So let's just
	 * start 32MB from the end, and put the primary header there.
	 * Don't do secondary for now.
	 * We don't know exactly where that will be yet as it could be
	 * different on each device.  So just set up the lengths.
	 */

	ddf->anchor.magic = DDF_HEADER_MAGIC;
	make_header_guid(ddf->anchor.guid);

	memcpy(ddf->anchor.revision, DDF_REVISION_2, 8);
	ddf->anchor.seq = cpu_to_be32(1);
	ddf->anchor.timestamp = cpu_to_be32(time(0) - DECADE);
	ddf->anchor.openflag = 0xFF;
	ddf->anchor.foreignflag = 0;
	ddf->anchor.enforcegroups = 0; /* Is this best?? */
	ddf->anchor.pad0 = 0xff;
	memset(ddf->anchor.pad1, 0xff, 12);
	memset(ddf->anchor.header_ext, 0xff, 32);
	ddf->anchor.primary_lba = cpu_to_be64(~(__u64)0);
	ddf->anchor.secondary_lba = cpu_to_be64(~(__u64)0);
	ddf->anchor.type = DDF_HEADER_ANCHOR;
	memset(ddf->anchor.pad2, 0xff, 3);
	ddf->anchor.workspace_len = cpu_to_be32(32768); /* Must be reserved */
	/* Put this at bottom of 32M reserved.. */
	ddf->anchor.workspace_lba = cpu_to_be64(~(__u64)0);
	max_phys_disks = 1023;   /* Should be enough, 4095 is also allowed */
	ddf->anchor.max_pd_entries = cpu_to_be16(max_phys_disks);
	max_virt_disks = 255; /* 15, 63, 255, 1024, 4095 are all allowed */
	ddf->anchor.max_vd_entries = cpu_to_be16(max_virt_disks);
	ddf->max_part = 64;
	ddf->anchor.max_partitions = cpu_to_be16(ddf->max_part);
	ddf->mppe = 256; /* 16, 64, 256, 1024, 4096 are all allowed */
	ddf->conf_rec_len = 1 + ROUND_UP(ddf->mppe * (4+8), 512)/512;
	ddf->anchor.config_record_len = cpu_to_be16(ddf->conf_rec_len);
	ddf->anchor.max_primary_element_entries = cpu_to_be16(ddf->mppe);
	memset(ddf->anchor.pad3, 0xff, 54);
	/* Controller section is one sector long immediately
	 * after the ddf header */
	sector = 1;
	ddf->anchor.controller_section_offset = cpu_to_be32(sector);
	ddf->anchor.controller_section_length = cpu_to_be32(1);
	sector += 1;

	/* phys is 8 sectors after that */
	pdsize = ROUND_UP(sizeof(struct phys_disk) +
			  sizeof(struct phys_disk_entry)*max_phys_disks,
			  512);
	switch(pdsize/512) {
	case 2: case 8: case 32: case 128: case 512: break;
	default: abort();
	}
	ddf->anchor.phys_section_offset = cpu_to_be32(sector);
	ddf->anchor.phys_section_length =
		cpu_to_be32(pdsize/512); /* max_primary_element_entries/8 */
	sector += pdsize/512;

	/* virt is another 32 sectors */
	vdsize = ROUND_UP(sizeof(struct virtual_disk) +
			  sizeof(struct virtual_entry) * max_virt_disks,
			  512);
	switch(vdsize/512) {
	case 2: case 8: case 32: case 128: case 512: break;
	default: abort();
	}
	ddf->anchor.virt_section_offset = cpu_to_be32(sector);
	ddf->anchor.virt_section_length =
		cpu_to_be32(vdsize/512); /* max_vd_entries/8 */
	sector += vdsize/512;

	clen = ddf->conf_rec_len * (ddf->max_part+1);
	ddf->anchor.config_section_offset = cpu_to_be32(sector);
	ddf->anchor.config_section_length = cpu_to_be32(clen);
	sector += clen;

	ddf->anchor.data_section_offset = cpu_to_be32(sector);
	ddf->anchor.data_section_length = cpu_to_be32(1);
	sector += 1;

	ddf->anchor.bbm_section_length = cpu_to_be32(0);
	ddf->anchor.bbm_section_offset = cpu_to_be32(0xFFFFFFFF);
	ddf->anchor.diag_space_length = cpu_to_be32(0);
	ddf->anchor.diag_space_offset = cpu_to_be32(0xFFFFFFFF);
	ddf->anchor.vendor_length = cpu_to_be32(0);
	ddf->anchor.vendor_offset = cpu_to_be32(0xFFFFFFFF);

	memset(ddf->anchor.pad4, 0xff, 256);

	memcpy(&ddf->primary, &ddf->anchor, 512);
	memcpy(&ddf->secondary, &ddf->anchor, 512);

	ddf->primary.openflag = 1; /* I guess.. */
	ddf->primary.type = DDF_HEADER_PRIMARY;

	ddf->secondary.openflag = 1; /* I guess.. */
	ddf->secondary.type = DDF_HEADER_SECONDARY;

	ddf->active = &ddf->primary;

	ddf->controller.magic = DDF_CONTROLLER_MAGIC;

	/* 24 more bytes of fiction required.
	 * first 8 are a 'vendor-id'  - "Linux-MD"
	 * Remaining 16 are serial number.... maybe a hostname would do?
	 */
	memcpy(ddf->controller.guid, T10, sizeof(T10));
	gethostname(hostname, sizeof(hostname));
	hostname[sizeof(hostname) - 1] = 0;
	hostlen = strlen(hostname);
	memcpy(ddf->controller.guid + 24 - hostlen, hostname, hostlen);
	for (i = strlen(T10) ; i+hostlen < 24; i++)
		ddf->controller.guid[i] = ' ';

	ddf->controller.type.vendor_id = cpu_to_be16(0xDEAD);
	ddf->controller.type.device_id = cpu_to_be16(0xBEEF);
	ddf->controller.type.sub_vendor_id = cpu_to_be16(0);
	ddf->controller.type.sub_device_id = cpu_to_be16(0);
	memcpy(ddf->controller.product_id, "What Is My PID??", 16);
	memset(ddf->controller.pad, 0xff, 8);
	memset(ddf->controller.vendor_data, 0xff, 448);
	if (homehost && strlen(homehost) < 440)
		strcpy((char*)ddf->controller.vendor_data, homehost);

	if (posix_memalign((void**)&pd, 512, pdsize) != 0) {
		pr_err("could not allocate pd\n");
		return 0;
	}
	ddf->phys = pd;
	ddf->pdsize = pdsize;

	memset(pd, 0xff, pdsize);
	memset(pd, 0, sizeof(*pd));
	pd->magic = DDF_PHYS_RECORDS_MAGIC;
	pd->used_pdes = cpu_to_be16(0);
	pd->max_pdes = cpu_to_be16(max_phys_disks);
	memset(pd->pad, 0xff, 52);
	for (i = 0; i < max_phys_disks; i++)
		memset(pd->entries[i].guid, 0xff, DDF_GUID_LEN);

	if (posix_memalign((void**)&vd, 512, vdsize) != 0) {
		pr_err("could not allocate vd\n");
		return 0;
	}
	ddf->virt = vd;
	ddf->vdsize = vdsize;
	memset(vd, 0, vdsize);
	vd->magic = DDF_VIRT_RECORDS_MAGIC;
	vd->populated_vdes = cpu_to_be16(0);
	vd->max_vdes = cpu_to_be16(max_virt_disks);
	memset(vd->pad, 0xff, 52);

	for (i=0; i<max_virt_disks; i++)
		memset(&vd->entries[i], 0xff, sizeof(struct virtual_entry));

	st->sb = ddf;
	ddf_set_updates_pending(ddf, NULL);
	return 1;
}

static int chunk_to_shift(int chunksize)
{
	return ffs(chunksize/512)-1;
}

struct extent {
	unsigned long long start, size;
};
static int cmp_extent(const void *av, const void *bv)
{
	const struct extent *a = av;
	const struct extent *b = bv;
	if (a->start < b->start)
		return -1;
	if (a->start > b->start)
		return 1;
	return 0;
}

static struct extent *get_extents(struct ddf_super *ddf, struct dl *dl)
{
	/* Find a list of used extents on the given physical device
	 * (dnum) of the given ddf.
	 * Return a malloced array of 'struct extent'
	 */
	struct extent *rv;
	int n = 0;
	unsigned int i;
	__u16 state;

	if (dl->pdnum < 0)
		return NULL;
	state = be16_to_cpu(ddf->phys->entries[dl->pdnum].state);

	if ((state & (DDF_Online|DDF_Failed|DDF_Missing)) != DDF_Online)
		return NULL;

	rv = xmalloc(sizeof(struct extent) * (ddf->max_part + 2));

	for (i = 0; i < ddf->max_part; i++) {
		const struct vd_config *bvd;
		unsigned int ibvd;
		struct vcl *v = dl->vlist[i];
		if (v == NULL ||
		    get_pd_index_from_refnum(v, dl->disk.refnum, ddf->mppe,
					     &bvd, &ibvd) == DDF_NOTFOUND)
			continue;
		rv[n].start = be64_to_cpu(LBA_OFFSET(ddf, bvd)[ibvd]);
		rv[n].size = be64_to_cpu(bvd->blocks);
		n++;
	}
	qsort(rv, n, sizeof(*rv), cmp_extent);

	rv[n].start = be64_to_cpu(ddf->phys->entries[dl->pdnum].config_size);
	rv[n].size = 0;
	return rv;
}

static unsigned long long find_space(
	struct ddf_super *ddf, struct dl *dl,
	unsigned long long data_offset,
	unsigned long long *size)
{
	/* Find if the requested amount of space is available.
	 * If it is, return start.
	 * If not, set *size to largest space.
	 * If data_offset != INVALID_SECTORS, then the space must start
	 * at this location.
	 */
	struct extent *e = get_extents(ddf, dl);
	int i = 0;
	unsigned long long pos = 0;
	unsigned long long max_size = 0;

	if (!e) {
		*size = 0;
		return INVALID_SECTORS;
	}
	do {
		unsigned long long esize = e[i].start - pos;
		if (data_offset != INVALID_SECTORS &&
		    pos <= data_offset &&
		    e[i].start > data_offset) {
			pos = data_offset;
			esize = e[i].start - pos;
		}
		if (data_offset != INVALID_SECTORS &&
		    pos != data_offset) {
			i++;
			continue;
		}
		if (esize >= *size) {
			/* Found! */
			free(e);
			return pos;
		}
		if (esize > max_size)
			max_size = esize;
		pos = e[i].start + e[i].size;
		i++;
	} while (e[i-1].size);
	*size = max_size;
	free(e);
	return INVALID_SECTORS;
}

static int init_super_ddf_bvd(struct supertype *st,
			      mdu_array_info_t *info,
			      unsigned long long size,
			      char *name, char *homehost,
			      int *uuid, unsigned long long data_offset)
{
	/* We are creating a BVD inside a pre-existing container.
	 * so st->sb is already set.
	 * We need to create a new vd_config and a new virtual_entry
	 */
	struct ddf_super *ddf = st->sb;
	unsigned int venum, i;
	struct virtual_entry *ve;
	struct vcl *vcl;
	struct vd_config *vc;

	if (find_vde_by_name(ddf, name) != DDF_NOTFOUND) {
		pr_err("This ddf already has an array called %s\n", name);
		return 0;
	}
	venum = find_unused_vde(ddf);
	if (venum == DDF_NOTFOUND) {
		pr_err("Cannot find spare slot for virtual disk\n");
		return 0;
	}
	ve = &ddf->virt->entries[venum];

	/* A Virtual Disk GUID contains the T10 Vendor ID, controller type,
	 * timestamp, random number
	 */
	make_header_guid(ve->guid);
	ve->unit = cpu_to_be16(info->md_minor);
	ve->pad0 = 0xFFFF;
	ve->guid_crc._v16 = crc32(0, (unsigned char *)ddf->anchor.guid,
				  DDF_GUID_LEN);
	ve->type = cpu_to_be16(0);
	ve->state = DDF_state_degraded; /* Will be modified as devices are added */
	if (info->state & 1) /* clean */
		ve->init_state = DDF_init_full;
	else
		ve->init_state = DDF_init_not;

	memset(ve->pad1, 0xff, 14);
	memset(ve->name, ' ', 16);
	if (name)
		strncpy(ve->name, name, 16);
	ddf->virt->populated_vdes =
		cpu_to_be16(be16_to_cpu(ddf->virt->populated_vdes)+1);

	/* Now create a new vd_config */
	if (posix_memalign((void**)&vcl, 512,
		           (offsetof(struct vcl, conf) + ddf->conf_rec_len * 512)) != 0) {
		pr_err("could not allocate vd_config\n");
		return 0;
	}
	vcl->vcnum = venum;
	vcl->block_sizes = NULL; /* FIXME not for CONCAT */
	vc = &vcl->conf;

	vc->magic = DDF_VD_CONF_MAGIC;
	memcpy(vc->guid, ve->guid, DDF_GUID_LEN);
	vc->timestamp = cpu_to_be32(time(0)-DECADE);
	vc->seqnum = cpu_to_be32(1);
	memset(vc->pad0, 0xff, 24);
	vc->chunk_shift = chunk_to_shift(info->chunk_size);
	if (layout_md2ddf(info, vc) == -1 ||
		be16_to_cpu(vc->prim_elmnt_count) > ddf->mppe) {
		pr_err("unsupported RAID level/layout %d/%d with %d disks\n",
		       info->level, info->layout, info->raid_disks);
		free(vcl);
		return 0;
	}
	vc->sec_elmnt_seq = 0;
	if (alloc_other_bvds(ddf, vcl) != 0) {
		pr_err("could not allocate other bvds\n");
		free(vcl);
		return 0;
	}
	vc->blocks = cpu_to_be64(size * 2);
	vc->array_blocks = cpu_to_be64(
		calc_array_size(info->level, info->raid_disks, info->layout,
				info->chunk_size, size * 2));
	memset(vc->pad1, 0xff, 8);
	vc->spare_refs[0] = cpu_to_be32(0xffffffff);
	vc->spare_refs[1] = cpu_to_be32(0xffffffff);
	vc->spare_refs[2] = cpu_to_be32(0xffffffff);
	vc->spare_refs[3] = cpu_to_be32(0xffffffff);
	vc->spare_refs[4] = cpu_to_be32(0xffffffff);
	vc->spare_refs[5] = cpu_to_be32(0xffffffff);
	vc->spare_refs[6] = cpu_to_be32(0xffffffff);
	vc->spare_refs[7] = cpu_to_be32(0xffffffff);
	memset(vc->cache_pol, 0, 8);
	vc->bg_rate = 0x80;
	memset(vc->pad2, 0xff, 3);
	memset(vc->pad3, 0xff, 52);
	memset(vc->pad4, 0xff, 192);
	memset(vc->v0, 0xff, 32);
	memset(vc->v1, 0xff, 32);
	memset(vc->v2, 0xff, 16);
	memset(vc->v3, 0xff, 16);
	memset(vc->vendor, 0xff, 32);

	memset(vc->phys_refnum, 0xff, 4*ddf->mppe);
	memset(vc->phys_refnum+ddf->mppe, 0x00, 8*ddf->mppe);

	for (i = 1; i < vc->sec_elmnt_count; i++) {
		memcpy(vcl->other_bvds[i-1], vc, ddf->conf_rec_len * 512);
		vcl->other_bvds[i-1]->sec_elmnt_seq = i;
	}

	vcl->next = ddf->conflist;
	ddf->conflist = vcl;
	ddf->currentconf = vcl;
	ddf_set_updates_pending(ddf, NULL);
	return 1;
}

static void add_to_super_ddf_bvd(struct supertype *st,
				 mdu_disk_info_t *dk, int fd, char *devname,
				 unsigned long long data_offset)
{
	/* fd and devname identify a device within the ddf container (st).
	 * dk identifies a location in the new BVD.
	 * We need to find suitable free space in that device and update
	 * the phys_refnum and lba_offset for the newly created vd_config.
	 * We might also want to update the type in the phys_disk
	 * section.
	 *
	 * Alternately: fd == -1 and we have already chosen which device to
	 * use and recorded in dlist->raid_disk;
	 */
	struct dl *dl;
	struct ddf_super *ddf = st->sb;
	struct vd_config *vc;
	unsigned int i;
	unsigned long long blocks, pos;
	unsigned int raid_disk = dk->raid_disk;

	if (fd == -1) {
		for (dl = ddf->dlist; dl ; dl = dl->next)
			if (dl->raiddisk == dk->raid_disk)
				break;
	} else {
		for (dl = ddf->dlist; dl ; dl = dl->next)
			if (dl->major == dk->major &&
			    dl->minor == dk->minor)
				break;
	}
	if (!dl || dl->pdnum < 0 || ! (dk->state & (1<<MD_DISK_SYNC)))
		return;

	vc = &ddf->currentconf->conf;
	if (vc->sec_elmnt_count > 1) {
		unsigned int n = be16_to_cpu(vc->prim_elmnt_count);
		if (raid_disk >= n)
			vc = ddf->currentconf->other_bvds[raid_disk / n - 1];
		raid_disk %= n;
	}

	blocks = be64_to_cpu(vc->blocks);
	if (ddf->currentconf->block_sizes)
		blocks = ddf->currentconf->block_sizes[dk->raid_disk];

	pos = find_space(ddf, dl, data_offset, &blocks);
	if (pos == INVALID_SECTORS)
		return;

	ddf->currentdev = dk->raid_disk;
	vc->phys_refnum[raid_disk] = dl->disk.refnum;
	LBA_OFFSET(ddf, vc)[raid_disk] = cpu_to_be64(pos);

	for (i = 0; i < ddf->max_part ; i++)
		if (dl->vlist[i] == NULL)
			break;
	if (i == ddf->max_part)
		return;
	dl->vlist[i] = ddf->currentconf;

	if (fd >= 0)
		dl->fd = fd;
	if (devname)
		dl->devname = devname;

	/* Check if we can mark array as optimal yet */
	i = ddf->currentconf->vcnum;
	ddf->virt->entries[i].state =
		(ddf->virt->entries[i].state & ~DDF_state_mask)
		| get_svd_state(ddf, ddf->currentconf);
	be16_clear(ddf->phys->entries[dl->pdnum].type,
		   cpu_to_be16(DDF_Global_Spare));
	be16_set(ddf->phys->entries[dl->pdnum].type,
		 cpu_to_be16(DDF_Active_in_VD));
	dprintf("added disk %d/%08x to VD %d/%s as disk %d\n",
		dl->pdnum, be32_to_cpu(dl->disk.refnum),
		ddf->currentconf->vcnum, guid_str(vc->guid),
		dk->raid_disk);
	ddf_set_updates_pending(ddf, vc);
}

static unsigned int find_unused_pde(const struct ddf_super *ddf)
{
	unsigned int i;
	for (i = 0; i < be16_to_cpu(ddf->phys->max_pdes); i++) {
		if (all_ff(ddf->phys->entries[i].guid))
			return i;
	}
	return DDF_NOTFOUND;
}

static void _set_config_size(struct phys_disk_entry *pde, const struct dl *dl)
{
	__u64 cfs, t;
	cfs = min(dl->size - 32*1024*2ULL, be64_to_cpu(dl->primary_lba));
	t = be64_to_cpu(dl->secondary_lba);
	if (t != ~(__u64)0)
		cfs = min(cfs, t);
	/*
	 * Some vendor DDF structures interpret workspace_lba
	 * very differently than we do: Make a sanity check on the value.
	 */
	t = be64_to_cpu(dl->workspace_lba);
	if (t < cfs) {
		__u64 wsp = cfs - t;
		if (wsp > 1024*1024*2ULL && wsp > dl->size / 16) {
			pr_err("%x:%x: workspace size 0x%llx too big, ignoring\n",
			       dl->major, dl->minor, (unsigned long long)wsp);
		} else
			cfs = t;
	}
	pde->config_size = cpu_to_be64(cfs);
	dprintf("%x:%x config_size %llx, DDF structure is %llx blocks\n",
		dl->major, dl->minor,
		(unsigned long long)cfs, (unsigned long long)(dl->size-cfs));
}

/* Add a device to a container, either while creating it or while
 * expanding a pre-existing container
 */
static int add_to_super_ddf(struct supertype *st,
			    mdu_disk_info_t *dk, int fd, char *devname,
			    unsigned long long data_offset)
{
	struct ddf_super *ddf = st->sb;
	struct dl *dd;
	time_t now;
	struct tm *tm;
	unsigned long long size;
	struct phys_disk_entry *pde;
	unsigned int n, i;
	struct stat stb;
	__u32 *tptr;

	if (ddf->currentconf) {
		add_to_super_ddf_bvd(st, dk, fd, devname, data_offset);
		return 0;
	}

	/* This is device numbered dk->number.  We need to create
	 * a phys_disk entry and a more detailed disk_data entry.
	 */
	fstat(fd, &stb);
	n = find_unused_pde(ddf);
	if (n == DDF_NOTFOUND) {
		pr_err("No free slot in array, cannot add disk\n");
		return 1;
	}
	pde = &ddf->phys->entries[n];
	get_dev_size(fd, NULL, &size);
	if (size <= 32*1024*1024) {
		pr_err("device size must be at least 32MB\n");
		return 1;
	}
	size >>= 9;

	if (posix_memalign((void**)&dd, 512,
		           sizeof(*dd) + sizeof(dd->vlist[0]) * ddf->max_part) != 0) {
		pr_err("could allocate buffer for new disk, aborting\n");
		return 1;
	}
	dd->major = major(stb.st_rdev);
	dd->minor = minor(stb.st_rdev);
	dd->devname = devname;
	dd->fd = fd;
	dd->spare = NULL;

	dd->disk.magic = DDF_PHYS_DATA_MAGIC;
	now = time(0);
	tm = localtime(&now);
	sprintf(dd->disk.guid, "%8s%04d%02d%02d", T10,
		(__u16)tm->tm_year+1900,
		(__u8)tm->tm_mon+1, (__u8)tm->tm_mday);
	tptr = (__u32 *)(dd->disk.guid + 16);
	*tptr++ = random32();
	*tptr = random32();

	do {
		/* Cannot be bothered finding a CRC of some irrelevant details*/
		dd->disk.refnum._v32 = random32();
		for (i = be16_to_cpu(ddf->active->max_pd_entries);
		     i > 0; i--)
			if (be32_eq(ddf->phys->entries[i-1].refnum,
				    dd->disk.refnum))
				break;
	} while (i > 0);

	dd->disk.forced_ref = 1;
	dd->disk.forced_guid = 1;
	memset(dd->disk.vendor, ' ', 32);
	memcpy(dd->disk.vendor, "Linux", 5);
	memset(dd->disk.pad, 0xff, 442);
	for (i = 0; i < ddf->max_part ; i++)
		dd->vlist[i] = NULL;

	dd->pdnum = n;

	if (st->update_tail) {
		int len = (sizeof(struct phys_disk) +
			   sizeof(struct phys_disk_entry));
		struct phys_disk *pd;

		pd = xmalloc(len);
		pd->magic = DDF_PHYS_RECORDS_MAGIC;
		pd->used_pdes = cpu_to_be16(n);
		pde = &pd->entries[0];
		dd->mdupdate = pd;
	} else
		ddf->phys->used_pdes = cpu_to_be16(
			1 + be16_to_cpu(ddf->phys->used_pdes));

	memcpy(pde->guid, dd->disk.guid, DDF_GUID_LEN);
	pde->refnum = dd->disk.refnum;
	pde->type = cpu_to_be16(DDF_Forced_PD_GUID | DDF_Global_Spare);
	pde->state = cpu_to_be16(DDF_Online);
	dd->size = size;
	/*
	 * If there is already a device in dlist, try to reserve the same
	 * amount of workspace. Otherwise, use 32MB.
	 * We checked disk size above already.
	 */
#define __calc_lba(new, old, lba, mb) do { \
		unsigned long long dif; \
		if ((old) != NULL) \
			dif = (old)->size - be64_to_cpu((old)->lba); \
		else \
			dif = (new)->size; \
		if ((new)->size > dif) \
			(new)->lba = cpu_to_be64((new)->size - dif); \
		else \
			(new)->lba = cpu_to_be64((new)->size - (mb*1024*2)); \
	} while (0)
	__calc_lba(dd, ddf->dlist, workspace_lba, 32);
	__calc_lba(dd, ddf->dlist, primary_lba, 16);
	if (ddf->dlist == NULL ||
	    be64_to_cpu(ddf->dlist->secondary_lba) != ~(__u64)0)
		__calc_lba(dd, ddf->dlist, secondary_lba, 32);
	_set_config_size(pde, dd);

	sprintf(pde->path, "%17.17s","Information: nil") ;
	memset(pde->pad, 0xff, 6);

	if (st->update_tail) {
		dd->next = ddf->add_list;
		ddf->add_list = dd;
	} else {
		dd->next = ddf->dlist;
		ddf->dlist = dd;
		ddf_set_updates_pending(ddf, NULL);
	}

	return 0;
}

static int remove_from_super_ddf(struct supertype *st, mdu_disk_info_t *dk)
{
	struct ddf_super *ddf = st->sb;
	struct dl *dl;

	/* mdmon has noticed that this disk (dk->major/dk->minor) has
	 * disappeared from the container.
	 * We need to arrange that it disappears from the metadata and
	 * internal data structures too.
	 * Most of the work is done by ddf_process_update which edits
	 * the metadata and closes the file handle and attaches the memory
	 * where free_updates will free it.
	 */
	for (dl = ddf->dlist; dl ; dl = dl->next)
		if (dl->major == dk->major &&
		    dl->minor == dk->minor)
			break;
	if (!dl || dl->pdnum < 0)
		return -1;

	if (st->update_tail) {
		int len = (sizeof(struct phys_disk) +
			   sizeof(struct phys_disk_entry));
		struct phys_disk *pd;

		pd = xmalloc(len);
		pd->magic = DDF_PHYS_RECORDS_MAGIC;
		pd->used_pdes = cpu_to_be16(dl->pdnum);
		pd->entries[0].state = cpu_to_be16(DDF_Missing);
		append_metadata_update(st, pd, len);
	}
	return 0;
}

/*
 * This is the write_init_super method for a ddf container.  It is
 * called when creating a container or adding another device to a
 * container.
 */

static int __write_ddf_structure(struct dl *d, struct ddf_super *ddf, __u8 type)
{
	unsigned long long sector;
	struct ddf_header *header;
	int fd, i, n_config, conf_size, buf_size;
	int ret = 0;
	char *conf;

	fd = d->fd;

	switch (type) {
	case DDF_HEADER_PRIMARY:
		header = &ddf->primary;
		sector = be64_to_cpu(header->primary_lba);
		break;
	case DDF_HEADER_SECONDARY:
		header = &ddf->secondary;
		sector = be64_to_cpu(header->secondary_lba);
		break;
	default:
		return 0;
	}
	if (sector == ~(__u64)0)
		return 0;

	header->type = type;
	header->openflag = 1;
	header->crc = calc_crc(header, 512);

	lseek64(fd, sector<<9, 0);
	if (write(fd, header, 512) < 0)
		goto out;

	ddf->controller.crc = calc_crc(&ddf->controller, 512);
	if (write(fd, &ddf->controller, 512) < 0)
		goto out;

	ddf->phys->crc = calc_crc(ddf->phys, ddf->pdsize);
	if (write(fd, ddf->phys, ddf->pdsize) < 0)
		goto out;
	ddf->virt->crc = calc_crc(ddf->virt, ddf->vdsize);
	if (write(fd, ddf->virt, ddf->vdsize) < 0)
		goto out;

	/* Now write lots of config records. */
	n_config = ddf->max_part;
	conf_size = ddf->conf_rec_len * 512;
	conf = ddf->conf;
	buf_size = conf_size * (n_config + 1);
	if (!conf) {
		if (posix_memalign((void**)&conf, 512, buf_size) != 0)
			goto out;
		ddf->conf = conf;
	}
	for (i = 0 ; i <= n_config ; i++) {
		struct vcl *c;
		struct vd_config *vdc = NULL;
		if (i == n_config) {
			c = (struct vcl *)d->spare;
			if (c)
				vdc = &c->conf;
		} else {
			unsigned int dummy;
			c = d->vlist[i];
			if (c)
				get_pd_index_from_refnum(
					c, d->disk.refnum,
					ddf->mppe,
					(const struct vd_config **)&vdc,
					&dummy);
		}
		if (vdc) {
			dprintf("writing conf record %i on disk %08x for %s/%u\n",
				i, be32_to_cpu(d->disk.refnum),
				guid_str(vdc->guid),
				vdc->sec_elmnt_seq);
			vdc->crc = calc_crc(vdc, conf_size);
			memcpy(conf + i*conf_size, vdc, conf_size);
		} else
			memset(conf + i*conf_size, 0xff, conf_size);
	}
	if (write(fd, conf, buf_size) != buf_size)
		goto out;

	d->disk.crc = calc_crc(&d->disk, 512);
	if (write(fd, &d->disk, 512) < 0)
		goto out;

	ret = 1;
out:
	header->openflag = 0;
	header->crc = calc_crc(header, 512);

	lseek64(fd, sector<<9, 0);
	if (write(fd, header, 512) < 0)
		ret = 0;

	return ret;
}

static int _write_super_to_disk(struct ddf_super *ddf, struct dl *d)
{
	unsigned long long size;
	int fd = d->fd;
	if (fd < 0)
		return 0;

	/* We need to fill in the primary, (secondary) and workspace
	 * lba's in the headers, set their checksums,
	 * Also checksum phys, virt....
	 *
	 * Then write everything out, finally the anchor is written.
	 */
	get_dev_size(fd, NULL, &size);
	size /= 512;
	memcpy(&ddf->anchor, ddf->active, 512);
	if (be64_to_cpu(d->workspace_lba) != 0ULL)
		ddf->anchor.workspace_lba = d->workspace_lba;
	else
		ddf->anchor.workspace_lba =
			cpu_to_be64(size - 32*1024*2);
	if (be64_to_cpu(d->primary_lba) != 0ULL)
		ddf->anchor.primary_lba = d->primary_lba;
	else
		ddf->anchor.primary_lba =
			cpu_to_be64(size - 16*1024*2);
	if (be64_to_cpu(d->secondary_lba) != 0ULL)
		ddf->anchor.secondary_lba = d->secondary_lba;
	else
		ddf->anchor.secondary_lba =
			cpu_to_be64(size - 32*1024*2);
	ddf->anchor.timestamp = cpu_to_be32(time(0) - DECADE);
	memcpy(&ddf->primary, &ddf->anchor, 512);
	memcpy(&ddf->secondary, &ddf->anchor, 512);

	ddf->anchor.type = DDF_HEADER_ANCHOR;
	ddf->anchor.openflag = 0xFF; /* 'open' means nothing */
	ddf->anchor.seq = cpu_to_be32(0xFFFFFFFF); /* no sequencing in anchor */
	ddf->anchor.crc = calc_crc(&ddf->anchor, 512);

	if (!__write_ddf_structure(d, ddf, DDF_HEADER_PRIMARY))
		return 0;

	if (!__write_ddf_structure(d, ddf, DDF_HEADER_SECONDARY))
		return 0;

	lseek64(fd, (size-1)*512, SEEK_SET);
	if (write(fd, &ddf->anchor, 512) < 0)
		return 0;

	return 1;
}

static int __write_init_super_ddf(struct supertype *st)
{
	struct ddf_super *ddf = st->sb;
	struct dl *d;
	int attempts = 0;
	int successes = 0;

	pr_state(ddf, __func__);

	/* try to write updated metadata,
	 * if we catch a failure move on to the next disk
	 */
	for (d = ddf->dlist; d; d=d->next) {
		attempts++;
		successes += _write_super_to_disk(ddf, d);
	}

	return attempts != successes;
}

static int write_init_super_ddf(struct supertype *st)
{
	struct ddf_super *ddf = st->sb;
	struct vcl *currentconf = ddf->currentconf;

	/* We are done with currentconf - reset it so st refers to the container */
	ddf->currentconf = NULL;

	if (st->update_tail) {
		/* queue the virtual_disk and vd_config as metadata updates */
		struct virtual_disk *vd;
		struct vd_config *vc;
		int len, tlen;
		unsigned int i;

		if (!currentconf) {
			/* Must be adding a physical disk to the container */
			int len = (sizeof(struct phys_disk) +
				   sizeof(struct phys_disk_entry));

			/* adding a disk to the container. */
			if (!ddf->add_list)
				return 0;

			append_metadata_update(st, ddf->add_list->mdupdate, len);
			ddf->add_list->mdupdate = NULL;
			return 0;
		}

		/* Newly created VD */

		/* First the virtual disk.  We have a slightly fake header */
		len = sizeof(struct virtual_disk) + sizeof(struct virtual_entry);
		vd = xmalloc(len);
		*vd = *ddf->virt;
		vd->entries[0] = ddf->virt->entries[currentconf->vcnum];
		vd->populated_vdes = cpu_to_be16(currentconf->vcnum);
		append_metadata_update(st, vd, len);

		/* Then the vd_config */
		len = ddf->conf_rec_len * 512;
		tlen = len * currentconf->conf.sec_elmnt_count;
		vc = xmalloc(tlen);
		memcpy(vc, &currentconf->conf, len);
		for (i = 1; i < currentconf->conf.sec_elmnt_count; i++)
			memcpy((char *)vc + i*len, currentconf->other_bvds[i-1],
			       len);
		append_metadata_update(st, vc, tlen);

		return 0;
	} else {
		struct dl *d;
		if (!currentconf)
			for (d = ddf->dlist; d; d=d->next)
				while (Kill(d->devname, NULL, 0, -1, 1) == 0);
		/* Note: we don't close the fd's now, but a subsequent
		 * ->free_super() will
		 */
		return __write_init_super_ddf(st);
	}
}

static __u64 avail_size_ddf(struct supertype *st, __u64 devsize,
			    unsigned long long data_offset)
{
	/* We must reserve the last 32Meg */
	if (devsize <= 32*1024*2)
		return 0;
	return devsize - 32*1024*2;
}

static int reserve_space(struct supertype *st, int raiddisks,
			 unsigned long long size, int chunk,
			 unsigned long long data_offset,
			 unsigned long long *freesize)
{
	/* Find 'raiddisks' spare extents at least 'size' big (but
	 * only caring about multiples of 'chunk') and remember
	 * them.   If size==0, find the largest size possible.
	 * Report available size in *freesize
	 * If space cannot be found, fail.
	 */
	struct dl *dl;
	struct ddf_super *ddf = st->sb;
	int cnt = 0;

	for (dl = ddf->dlist; dl ; dl=dl->next) {
		dl->raiddisk = -1;
		dl->esize = 0;
	}
	/* Now find largest extent on each device */
	for (dl = ddf->dlist ; dl ; dl=dl->next) {
		unsigned long long minsize = ULLONG_MAX;

		find_space(ddf, dl, data_offset, &minsize);
		if (minsize >= size && minsize >= (unsigned)chunk) {
			cnt++;
			dl->esize = minsize;
		}
	}
	if (cnt < raiddisks) {
		pr_err("not enough devices with space to create array.\n");
		return 0; /* No enough free spaces large enough */
	}
	if (size == 0) {
		/* choose the largest size of which there are at least 'raiddisk' */
		for (dl = ddf->dlist ; dl ; dl=dl->next) {
			struct dl *dl2;
			if (dl->esize <= size)
				continue;
			/* This is bigger than 'size', see if there are enough */
			cnt = 0;
			for (dl2 = ddf->dlist; dl2 ; dl2=dl2->next)
				if (dl2->esize >= dl->esize)
					cnt++;
			if (cnt >= raiddisks)
				size = dl->esize;
		}
		if (chunk) {
			size = size / chunk;
			size *= chunk;
		}
		*freesize = size;
		if (size < 32) {
			pr_err("not enough spare devices to create array.\n");
			return 0;
		}
	}
	/* We have a 'size' of which there are enough spaces.
	 * We simply do a first-fit */
	cnt = 0;
	for (dl = ddf->dlist ; dl && cnt < raiddisks ; dl=dl->next) {
		if (dl->esize < size)
			continue;

		dl->raiddisk = cnt;
		cnt++;
	}
	return 1;
}

static int validate_geometry_ddf(struct supertype *st,
				 int level, int layout, int raiddisks,
				 int *chunk, unsigned long long size,
				 unsigned long long data_offset,
				 char *dev, unsigned long long *freesize,
				 int consistency_policy, int verbose)
{
	int fd;
	struct mdinfo *sra;
	int cfd;

	/* ddf potentially supports lots of things, but it depends on
	 * what devices are offered (and maybe kernel version?)
	 * If given unused devices, we will make a container.
	 * If given devices in a container, we will make a BVD.
	 * If given BVDs, we make an SVD, changing all the GUIDs in the process.
	 */

	if (*chunk == UnSet)
		*chunk = DEFAULT_CHUNK;

	if (level == LEVEL_NONE)
		level = LEVEL_CONTAINER;
	if (level == LEVEL_CONTAINER) {
		/* Must be a fresh device to add to a container */
		return validate_geometry_ddf_container(st, level, layout,
						       raiddisks, *chunk,
						       size, data_offset, dev,
						       freesize,
						       verbose);
	}

	if (!dev) {
		mdu_array_info_t array = {
			.level = level,
			.layout = layout,
			.raid_disks = raiddisks
		};
		struct vd_config conf;
		if (layout_md2ddf(&array, &conf) == -1) {
			if (verbose)
				pr_err("DDF does not support level %d /layout %d arrays with %d disks\n",
				       level, layout, raiddisks);
			return 0;
		}
		/* Should check layout? etc */

		if (st->sb && freesize) {
			/* --create was given a container to create in.
			 * So we need to check that there are enough
			 * free spaces and return the amount of space.
			 * We may as well remember which drives were
			 * chosen so that add_to_super/getinfo_super
			 * can return them.
			 */
			return reserve_space(st, raiddisks, size, *chunk,
					     data_offset, freesize);
		}
		return 1;
	}

	if (st->sb) {
		/* A container has already been opened, so we are
		 * creating in there.  Maybe a BVD, maybe an SVD.
		 * Should make a distinction one day.
		 */
		return validate_geometry_ddf_bvd(st, level, layout, raiddisks,
						 chunk, size, data_offset, dev,
						 freesize,
						 verbose);
	}
	/* This is the first device for the array.
	 * If it is a container, we read it in and do automagic allocations,
	 * no other devices should be given.
	 * Otherwise it must be a member device of a container, and we
	 * do manual allocation.
	 * Later we should check for a BVD and make an SVD.
	 */
	fd = open(dev, O_RDONLY|O_EXCL, 0);
	if (fd >= 0) {
		close(fd);
		/* Just a bare device, no good to us */
		if (verbose)
			pr_err("ddf: Cannot create this array on device %s - a container is required.\n",
			       dev);
		return 0;
	}
	if (errno != EBUSY || (fd = open(dev, O_RDONLY, 0)) < 0) {
		if (verbose)
			pr_err("ddf: Cannot open %s: %s\n",
			       dev, strerror(errno));
		return 0;
	}
	/* Well, it is in use by someone, maybe a 'ddf' container. */
	cfd = open_container(fd);
	if (cfd < 0) {
		close(fd);
		if (verbose)
			pr_err("ddf: Cannot use %s: %s\n",
			       dev, strerror(EBUSY));
		return 0;
	}
	sra = sysfs_read(cfd, NULL, GET_VERSION);
	close(fd);
	if (sra && sra->array.major_version == -1 &&
	    strcmp(sra->text_version, "ddf") == 0) {
		/* This is a member of a ddf container.  Load the container
		 * and try to create a bvd
		 */
		struct ddf_super *ddf;
		if (load_super_ddf_all(st, cfd, (void **)&ddf, NULL) == 0) {
			st->sb = ddf;
			strcpy(st->container_devnm, fd2devnm(cfd));
			close(cfd);
			return validate_geometry_ddf_bvd(st, level, layout,
							 raiddisks, chunk, size,
							 data_offset,
							 dev, freesize,
							 verbose);
		}
		close(cfd);
	} else /* device may belong to a different container */
		return 0;

	return 1;
}

static int
validate_geometry_ddf_container(struct supertype *st,
				int level, int layout, int raiddisks,
				int chunk, unsigned long long size,
				unsigned long long data_offset,
				char *dev, unsigned long long *freesize,
				int verbose)
{
	int fd;
	unsigned long long ldsize;

	if (level != LEVEL_CONTAINER)
		return 0;
	if (!dev)
		return 1;

	fd = open(dev, O_RDONLY|O_EXCL, 0);
	if (fd < 0) {
		if (verbose)
			pr_err("ddf: Cannot open %s: %s\n",
			       dev, strerror(errno));
		return 0;
	}
	if (!get_dev_size(fd, dev, &ldsize)) {
		close(fd);
		return 0;
	}
	close(fd);

	*freesize = avail_size_ddf(st, ldsize >> 9, INVALID_SECTORS);
	if (*freesize == 0)
		return 0;

	return 1;
}

static int validate_geometry_ddf_bvd(struct supertype *st,
				     int level, int layout, int raiddisks,
				     int *chunk, unsigned long long size,
				     unsigned long long data_offset,
				     char *dev, unsigned long long *freesize,
				     int verbose)
{
	dev_t rdev;
	struct ddf_super *ddf = st->sb;
	struct dl *dl;
	unsigned long long maxsize;
	/* ddf/bvd supports lots of things, but not containers */
	if (level == LEVEL_CONTAINER) {
		if (verbose)
			pr_err("DDF cannot create a container within an container\n");
		return 0;
	}
	/* We must have the container info already read in. */
	if (!ddf)
		return 0;

	if (!dev) {
		/* General test:  make sure there is space for
		 * 'raiddisks' device extents of size 'size'.
		 */
		unsigned long long minsize = size;
		int dcnt = 0;
		if (minsize == 0)
			minsize = 8;
		for (dl = ddf->dlist; dl ; dl = dl->next) {
			if (find_space(ddf, dl, data_offset, &minsize) !=
			    INVALID_SECTORS)
				dcnt++;
		}
		if (dcnt < raiddisks) {
			if (verbose)
				pr_err("ddf: Not enough devices with space for this array (%d < %d)\n",
				       dcnt, raiddisks);
			return 0;
		}
		return 1;
	}
	/* This device must be a member of the set */
	if (!stat_is_blkdev(dev, &rdev))
		return 0;
	for (dl = ddf->dlist ; dl ; dl = dl->next) {
		if (dl->major == (int)major(rdev) &&
		    dl->minor == (int)minor(rdev))
			break;
	}
	if (!dl) {
		if (verbose)
			pr_err("ddf: %s is not in the same DDF set\n",
			       dev);
		return 0;
	}
	maxsize = ULLONG_MAX;
	find_space(ddf, dl, data_offset, &maxsize);
	*freesize = maxsize;

	return 1;
}

static int load_super_ddf_all(struct supertype *st, int fd,
			      void **sbp, char *devname)
{
	struct mdinfo *sra;
	struct ddf_super *super;
	struct mdinfo *sd, *best = NULL;
	int bestseq = 0;
	int seq;
	char nm[20];
	int dfd;

	sra = sysfs_read(fd, NULL, GET_LEVEL|GET_VERSION|GET_DEVS|GET_STATE);
	if (!sra)
		return 1;
	if (sra->array.major_version != -1 ||
	    sra->array.minor_version != -2 ||
	    strcmp(sra->text_version, "ddf") != 0)
		return 1;

	if (posix_memalign((void**)&super, 512, sizeof(*super)) != 0)
		return 1;
	memset(super, 0, sizeof(*super));

	/* first, try each device, and choose the best ddf */
	for (sd = sra->devs ; sd ; sd = sd->next) {
		int rv;
		sprintf(nm, "%d:%d", sd->disk.major, sd->disk.minor);
		dfd = dev_open(nm, O_RDONLY);
		if (dfd < 0)
			return 2;
		rv = load_ddf_headers(dfd, super, NULL);
		close(dfd);
		if (rv == 0) {
			seq = be32_to_cpu(super->active->seq);
			if (super->active->openflag)
				seq--;
			if (!best || seq > bestseq) {
				bestseq = seq;
				best = sd;
			}
		}
	}
	if (!best)
		return 1;
	/* OK, load this ddf */
	sprintf(nm, "%d:%d", best->disk.major, best->disk.minor);
	dfd = dev_open(nm, O_RDONLY);
	if (dfd < 0)
		return 1;
	load_ddf_headers(dfd, super, NULL);
	load_ddf_global(dfd, super, NULL);
	close(dfd);
	/* Now we need the device-local bits */
	for (sd = sra->devs ; sd ; sd = sd->next) {
		int rv;

		sprintf(nm, "%d:%d", sd->disk.major, sd->disk.minor);
		dfd = dev_open(nm, O_RDWR);
		if (dfd < 0)
			return 2;
		rv = load_ddf_headers(dfd, super, NULL);
		if (rv == 0)
			rv = load_ddf_local(dfd, super, NULL, 1);
		if (rv)
			return 1;
	}

	*sbp = super;
	if (st->ss == NULL) {
		st->ss = &super_ddf;
		st->minor_version = 0;
		st->max_devs = 512;
	}
	strcpy(st->container_devnm, fd2devnm(fd));
	return 0;
}

static int load_container_ddf(struct supertype *st, int fd,
			      char *devname)
{
	return load_super_ddf_all(st, fd, &st->sb, devname);
}

static int check_secondary(const struct vcl *vc)
{
	const struct vd_config *conf = &vc->conf;
	int i;

	/* The only DDF secondary RAID level md can support is
	 * RAID 10, if the stripe sizes and Basic volume sizes
	 * are all equal.
	 * Other configurations could in theory be supported by exposing
	 * the BVDs to user space and using device mapper for the secondary
	 * mapping. So far we don't support that.
	 */

	__u64 sec_elements[4] = {0, 0, 0, 0};
#define __set_sec_seen(n) (sec_elements[(n)>>6] |= (1<<((n)&63)))
#define __was_sec_seen(n) ((sec_elements[(n)>>6] & (1<<((n)&63))) != 0)

	if (vc->other_bvds == NULL) {
		pr_err("No BVDs for secondary RAID found\n");
		return -1;
	}
	if (conf->prl != DDF_RAID1) {
		pr_err("Secondary RAID level only supported for mirrored BVD\n");
		return -1;
	}
	if (conf->srl != DDF_2STRIPED && conf->srl != DDF_2SPANNED) {
		pr_err("Secondary RAID level %d is unsupported\n",
		       conf->srl);
		return -1;
	}
	__set_sec_seen(conf->sec_elmnt_seq);
	for (i = 0; i < conf->sec_elmnt_count-1; i++) {
		const struct vd_config *bvd = vc->other_bvds[i];
		if (bvd->sec_elmnt_seq == DDF_UNUSED_BVD)
			continue;
		if (bvd->srl != conf->srl) {
			pr_err("Inconsistent secondary RAID level across BVDs\n");
			return -1;
		}
		if (bvd->prl != conf->prl) {
			pr_err("Different RAID levels for BVDs are unsupported\n");
			return -1;
		}
		if (!be16_eq(bvd->prim_elmnt_count, conf->prim_elmnt_count)) {
			pr_err("All BVDs must have the same number of primary elements\n");
			return -1;
		}
		if (bvd->chunk_shift != conf->chunk_shift) {
			pr_err("Different strip sizes for BVDs are unsupported\n");
			return -1;
		}
		if (!be64_eq(bvd->array_blocks, conf->array_blocks)) {
			pr_err("Different BVD sizes are unsupported\n");
			return -1;
		}
		__set_sec_seen(bvd->sec_elmnt_seq);
	}
	for (i = 0; i < conf->sec_elmnt_count; i++) {
		if (!__was_sec_seen(i)) {
			/* pr_err("BVD %d is missing\n", i); */
			return -1;
		}
	}
	return 0;
}

static unsigned int get_pd_index_from_refnum(const struct vcl *vc,
					     be32 refnum, unsigned int nmax,
					     const struct vd_config **bvd,
					     unsigned int *idx)
{
	unsigned int i, j, n, sec, cnt;

	cnt = be16_to_cpu(vc->conf.prim_elmnt_count);
	sec = (vc->conf.sec_elmnt_count == 1 ? 0 : vc->conf.sec_elmnt_seq);

	for (i = 0, j = 0 ; i < nmax ; i++) {
		/* j counts valid entries for this BVD */
		if (be32_eq(vc->conf.phys_refnum[i], refnum)) {
			*bvd = &vc->conf;
			*idx = i;
			return sec * cnt + j;
		}
		if (be32_to_cpu(vc->conf.phys_refnum[i]) != 0xffffffff)
			j++;
	}
	if (vc->other_bvds == NULL)
		goto bad;

	for (n = 1; n < vc->conf.sec_elmnt_count; n++) {
		struct vd_config *vd = vc->other_bvds[n-1];
		sec = vd->sec_elmnt_seq;
		if (sec == DDF_UNUSED_BVD)
			continue;
		for (i = 0, j = 0 ; i < nmax ; i++) {
			if (be32_eq(vd->phys_refnum[i], refnum)) {
				*bvd = vd;
				*idx = i;
				return sec * cnt + j;
			}
			if (be32_to_cpu(vd->phys_refnum[i]) != 0xffffffff)
				j++;
		}
	}
bad:
	*bvd = NULL;
	return DDF_NOTFOUND;
}

static struct mdinfo *container_content_ddf(struct supertype *st, char *subarray)
{
	/* Given a container loaded by load_super_ddf_all,
	 * extract information about all the arrays into
	 * an mdinfo tree.
	 *
	 * For each vcl in conflist: create an mdinfo, fill it in,
	 *  then look for matching devices (phys_refnum) in dlist
	 *  and create appropriate device mdinfo.
	 */
	struct ddf_super *ddf = st->sb;
	struct mdinfo *rest = NULL;
	struct vcl *vc;

	for (vc = ddf->conflist ; vc ; vc=vc->next) {
		unsigned int i;
		struct mdinfo *this;
		char *ep;
		__u32 *cptr;
		unsigned int pd;

		if (subarray &&
		    (strtoul(subarray, &ep, 10) != vc->vcnum ||
		     *ep != '\0'))
			continue;

		if (vc->conf.sec_elmnt_count > 1) {
			if (check_secondary(vc) != 0)
				continue;
		}

		this = xcalloc(1, sizeof(*this));
		this->next = rest;
		rest = this;

		if (layout_ddf2md(&vc->conf, &this->array))
			continue;
		this->array.md_minor      = -1;
		this->array.major_version = -1;
		this->array.minor_version = -2;
		this->safe_mode_delay	  = DDF_SAFE_MODE_DELAY;
		cptr = (__u32 *)(vc->conf.guid + 16);
		this->array.ctime         = DECADE + __be32_to_cpu(*cptr);
		this->array.utime	  = DECADE +
			be32_to_cpu(vc->conf.timestamp);
		this->array.chunk_size	  = 512 << vc->conf.chunk_shift;

		i = vc->vcnum;
		if ((ddf->virt->entries[i].state & DDF_state_inconsistent) ||
		    (ddf->virt->entries[i].init_state & DDF_initstate_mask) !=
		    DDF_init_full) {
			this->array.state = 0;
			this->resync_start = 0;
		} else {
			this->array.state = 1;
			this->resync_start = MaxSector;
		}
		_ddf_array_name(this->name, ddf, i);
		memset(this->uuid, 0, sizeof(this->uuid));
		this->component_size	  = be64_to_cpu(vc->conf.blocks);
		this->array.size	  = this->component_size / 2;
		this->container_member	  = i;

		ddf->currentconf = vc;
		uuid_from_super_ddf(st, this->uuid);
		if (!subarray)
			ddf->currentconf = NULL;

		sprintf(this->text_version, "/%s/%d",
			st->container_devnm, this->container_member);

		for (pd = 0; pd < be16_to_cpu(ddf->phys->max_pdes); pd++) {
			struct mdinfo *dev;
			struct dl *d;
			const struct vd_config *bvd;
			unsigned int iphys;
			int stt;

			if (be32_to_cpu(ddf->phys->entries[pd].refnum) ==
			    0xffffffff)
				continue;

			stt = be16_to_cpu(ddf->phys->entries[pd].state);
			if ((stt & (DDF_Online|DDF_Failed|DDF_Rebuilding)) !=
			    DDF_Online)
				continue;

			i = get_pd_index_from_refnum(
				vc, ddf->phys->entries[pd].refnum,
				ddf->mppe, &bvd, &iphys);
			if (i == DDF_NOTFOUND)
				continue;

			this->array.working_disks++;

			for (d = ddf->dlist; d ; d=d->next)
				if (be32_eq(d->disk.refnum,
					    ddf->phys->entries[pd].refnum))
					break;
			if (d == NULL)
				/* Haven't found that one yet, maybe there are others */
				continue;

			dev = xcalloc(1, sizeof(*dev));
			dev->next	 = this->devs;
			this->devs	 = dev;

			dev->disk.number = be32_to_cpu(d->disk.refnum);
			dev->disk.major	 = d->major;
			dev->disk.minor	 = d->minor;
			dev->disk.raid_disk = i;
			dev->disk.state	 = (1<<MD_DISK_SYNC)|(1<<MD_DISK_ACTIVE);
			dev->recovery_start = MaxSector;

			dev->events	 = be32_to_cpu(ddf->active->seq);
			dev->data_offset =
				be64_to_cpu(LBA_OFFSET(ddf, bvd)[iphys]);
			dev->component_size = be64_to_cpu(bvd->blocks);
			if (d->devname)
				strcpy(dev->name, d->devname);
		}
	}
	return rest;
}

static int store_super_ddf(struct supertype *st, int fd)
{
	struct ddf_super *ddf = st->sb;
	unsigned long long dsize;
	void *buf;
	int rc;

	if (!ddf)
		return 1;

	if (!get_dev_size(fd, NULL, &dsize))
		return 1;

	if (ddf->dlist || ddf->conflist) {
		struct stat sta;
		struct dl *dl;
		int ofd, ret;

		if (fstat(fd, &sta) == -1 || !S_ISBLK(sta.st_mode)) {
			pr_err("file descriptor for invalid device\n");
			return 1;
		}
		for (dl = ddf->dlist; dl; dl = dl->next)
			if (dl->major == (int)major(sta.st_rdev) &&
			    dl->minor == (int)minor(sta.st_rdev))
				break;
		if (!dl) {
			pr_err("couldn't find disk %d/%d\n",
			       (int)major(sta.st_rdev),
			       (int)minor(sta.st_rdev));
			return 1;
		}
		ofd = dl->fd;
		dl->fd = fd;
		ret = (_write_super_to_disk(ddf, dl) != 1);
		dl->fd = ofd;
		return ret;
	}

	if (posix_memalign(&buf, 512, 512) != 0)
		return 1;
	memset(buf, 0, 512);

	lseek64(fd, dsize-512, 0);
	rc = write(fd, buf, 512);
	free(buf);
	if (rc < 0)
		return 1;
	return 0;
}

static int compare_super_ddf(struct supertype *st, struct supertype *tst)
{
	/*
	 * return:
	 *  0 same, or first was empty, and second was copied
	 *  1 second had wrong magic number - but that isn't possible
	 *  2 wrong uuid
	 *  3 wrong other info
	 */
	struct ddf_super *first = st->sb;
	struct ddf_super *second = tst->sb;
	struct dl *dl1, *dl2;
	struct vcl *vl1, *vl2;
	unsigned int max_vds, max_pds, pd, vd;

	if (!first) {
		st->sb = tst->sb;
		tst->sb = NULL;
		return 0;
	}

	if (memcmp(first->anchor.guid, second->anchor.guid, DDF_GUID_LEN) != 0)
		return 2;

	/* It is only OK to compare info in the anchor.  Anything else
	 * could be changing due to a reconfig so must be ignored.
	 * guid really should be enough anyway.
	 */

	if (!be32_eq(first->active->seq, second->active->seq)) {
		dprintf("sequence number mismatch %u<->%u\n",
			be32_to_cpu(first->active->seq),
			be32_to_cpu(second->active->seq));
		return 0;
	}

	/*
	 * At this point we are fairly sure that the meta data matches.
	 * But the new disk may contain additional local data.
	 * Add it to the super block.
	 */
	max_vds = be16_to_cpu(first->active->max_vd_entries);
	max_pds = be16_to_cpu(first->phys->max_pdes);
	for (vl2 = second->conflist; vl2; vl2 = vl2->next) {
		for (vl1 = first->conflist; vl1; vl1 = vl1->next)
			if (!memcmp(vl1->conf.guid, vl2->conf.guid,
				    DDF_GUID_LEN))
				break;
		if (vl1) {
			if (vl1->other_bvds != NULL &&
			    vl1->conf.sec_elmnt_seq !=
			    vl2->conf.sec_elmnt_seq) {
				dprintf("adding BVD %u\n",
					vl2->conf.sec_elmnt_seq);
				add_other_bvd(vl1, &vl2->conf,
					      first->conf_rec_len*512);
			}
			continue;
		}

		if (posix_memalign((void **)&vl1, 512,
				   (first->conf_rec_len*512 +
				    offsetof(struct vcl, conf))) != 0) {
			pr_err("could not allocate vcl buf\n");
			return 3;
		}

		vl1->next = first->conflist;
		vl1->block_sizes = NULL;
		memcpy(&vl1->conf, &vl2->conf, first->conf_rec_len*512);
		if (alloc_other_bvds(first, vl1) != 0) {
			pr_err("could not allocate other bvds\n");
			free(vl1);
			return 3;
		}
		for (vd = 0; vd < max_vds; vd++)
			if (!memcmp(first->virt->entries[vd].guid,
				    vl1->conf.guid, DDF_GUID_LEN))
				break;
		vl1->vcnum = vd;
		dprintf("added config for VD %u\n", vl1->vcnum);
		first->conflist = vl1;
	}

	for (dl2 = second->dlist; dl2; dl2 = dl2->next) {
		for (dl1 = first->dlist; dl1; dl1 = dl1->next)
			if (be32_eq(dl1->disk.refnum, dl2->disk.refnum))
				break;
		if (dl1)
			continue;

		if (posix_memalign((void **)&dl1, 512,
				   sizeof(*dl1) + (first->max_part) *
				   sizeof(dl1->vlist[0])) != 0) {
			pr_err("could not allocate disk info buffer\n");
			return 3;
		}
		memcpy(dl1, dl2, sizeof(*dl1));
		dl1->mdupdate = NULL;
		dl1->next = first->dlist;
		dl1->fd = -1;
		for (pd = 0; pd < max_pds; pd++)
			if (be32_eq(first->phys->entries[pd].refnum,
				    dl1->disk.refnum))
				break;
		dl1->pdnum = pd < max_pds ? (int)pd : -1;
		if (dl2->spare) {
			if (posix_memalign((void **)&dl1->spare, 512,
				       first->conf_rec_len*512) != 0) {
				pr_err("could not allocate spare info buf\n");
				return 3;
			}
			memcpy(dl1->spare, dl2->spare, first->conf_rec_len*512);
		}
		for (vd = 0 ; vd < first->max_part ; vd++) {
			if (!dl2->vlist[vd]) {
				dl1->vlist[vd] = NULL;
				continue;
			}
			for (vl1 = first->conflist; vl1; vl1 = vl1->next) {
				if (!memcmp(vl1->conf.guid,
					    dl2->vlist[vd]->conf.guid,
					    DDF_GUID_LEN))
					break;
				dl1->vlist[vd] = vl1;
			}
		}
		first->dlist = dl1;
		dprintf("added disk %d: %08x\n", dl1->pdnum,
			be32_to_cpu(dl1->disk.refnum));
	}

	return 0;
}

/*
 * A new array 'a' has been started which claims to be instance 'inst'
 * within container 'c'.
 * We need to confirm that the array matches the metadata in 'c' so
 * that we don't corrupt any metadata.
 */
static int ddf_open_new(struct supertype *c, struct active_array *a, char *inst)
{
	struct ddf_super *ddf = c->sb;
	int n = atoi(inst);
	struct mdinfo *dev;
	struct dl *dl;
	static const char faulty[] = "faulty";

	if (all_ff(ddf->virt->entries[n].guid)) {
		pr_err("subarray %d doesn't exist\n", n);
		return -ENODEV;
	}
	dprintf("new subarray %d, GUID: %s\n", n,
		guid_str(ddf->virt->entries[n].guid));
	for (dev = a->info.devs; dev; dev = dev->next) {
		for (dl = ddf->dlist; dl; dl = dl->next)
			if (dl->major == dev->disk.major &&
			    dl->minor == dev->disk.minor)
				break;
		if (!dl || dl->pdnum < 0) {
			pr_err("device %d/%d of subarray %d not found in meta data\n",
				dev->disk.major, dev->disk.minor, n);
			return -1;
		}
		if ((be16_to_cpu(ddf->phys->entries[dl->pdnum].state) &
			(DDF_Online|DDF_Missing|DDF_Failed)) != DDF_Online) {
			pr_err("new subarray %d contains broken device %d/%d (%02x)\n",
			       n, dl->major, dl->minor,
			       be16_to_cpu(ddf->phys->entries[dl->pdnum].state));
			if (write(dev->state_fd, faulty, sizeof(faulty)-1) !=
			    sizeof(faulty) - 1)
				pr_err("Write to state_fd failed\n");
			dev->curr_state = DS_FAULTY;
		}
	}
	a->info.container_member = n;
	return 0;
}

static void handle_missing(struct ddf_super *ddf, struct active_array *a, int inst)
{
	/* This member array is being activated.  If any devices
	 * are missing they must now be marked as failed.
	 */
	struct vd_config *vc;
	unsigned int n_bvd;
	struct vcl *vcl;
	struct dl *dl;
	int pd;
	int n;
	int state;

	for (n = 0; ; n++) {
		vc = find_vdcr(ddf, inst, n, &n_bvd, &vcl);
		if (!vc)
			break;
		for (dl = ddf->dlist; dl; dl = dl->next)
			if (be32_eq(dl->disk.refnum, vc->phys_refnum[n_bvd]))
				break;
		if (dl)
			/* Found this disk, so not missing */
			continue;

		/* Mark the device as failed/missing. */
		pd = find_phys(ddf, vc->phys_refnum[n_bvd]);
		if (pd >= 0 && be16_and(ddf->phys->entries[pd].state,
					cpu_to_be16(DDF_Online))) {
			be16_clear(ddf->phys->entries[pd].state,
				   cpu_to_be16(DDF_Online));
			be16_set(ddf->phys->entries[pd].state,
				 cpu_to_be16(DDF_Failed|DDF_Missing));
			vc->phys_refnum[n_bvd] = cpu_to_be32(0);
			ddf_set_updates_pending(ddf, vc);
		}

		/* Mark the array as Degraded */
		state = get_svd_state(ddf, vcl);
		if (ddf->virt->entries[inst].state !=
		    ((ddf->virt->entries[inst].state & ~DDF_state_mask)
		     | state)) {
			ddf->virt->entries[inst].state =
				(ddf->virt->entries[inst].state & ~DDF_state_mask)
				| state;
			a->check_degraded = 1;
			ddf_set_updates_pending(ddf, vc);
		}
	}
}

/*
 * The array 'a' is to be marked clean in the metadata.
 * If '->resync_start' is not ~(unsigned long long)0, then the array is only
 * clean up to the point (in sectors).  If that cannot be recorded in the
 * metadata, then leave it as dirty.
 *
 * For DDF, we need to clear the DDF_state_inconsistent bit in the
 * !global! virtual_disk.virtual_entry structure.
 */
static int ddf_set_array_state(struct active_array *a, int consistent)
{
	struct ddf_super *ddf = a->container->sb;
	int inst = a->info.container_member;
	int old = ddf->virt->entries[inst].state;
	if (consistent == 2) {
		handle_missing(ddf, a, inst);
		consistent = 1;
		if (!is_resync_complete(&a->info))
			consistent = 0;
	}
	if (consistent)
		ddf->virt->entries[inst].state &= ~DDF_state_inconsistent;
	else
		ddf->virt->entries[inst].state |= DDF_state_inconsistent;
	if (old != ddf->virt->entries[inst].state)
		ddf_set_updates_pending(ddf, NULL);

	old = ddf->virt->entries[inst].init_state;
	ddf->virt->entries[inst].init_state &= ~DDF_initstate_mask;
	if (is_resync_complete(&a->info))
		ddf->virt->entries[inst].init_state |= DDF_init_full;
	else if (a->info.resync_start == 0)
		ddf->virt->entries[inst].init_state |= DDF_init_not;
	else
		ddf->virt->entries[inst].init_state |= DDF_init_quick;
	if (old != ddf->virt->entries[inst].init_state)
		ddf_set_updates_pending(ddf, NULL);

	dprintf("ddf mark %d/%s (%d) %s %llu\n", inst,
		guid_str(ddf->virt->entries[inst].guid), a->curr_state,
		consistent?"clean":"dirty",
		a->info.resync_start);
	return consistent;
}

static int get_bvd_state(const struct ddf_super *ddf,
			 const struct vd_config *vc)
{
	unsigned int i, n_bvd, working = 0;
	unsigned int n_prim = be16_to_cpu(vc->prim_elmnt_count);
	int pd, st, state;
	char *avail = xcalloc(1, n_prim);
	mdu_array_info_t array;

	layout_ddf2md(vc, &array);

	for (i = 0; i < n_prim; i++) {
		if (!find_index_in_bvd(ddf, vc, i, &n_bvd))
			continue;
		pd = find_phys(ddf, vc->phys_refnum[n_bvd]);
		if (pd < 0)
			continue;
		st = be16_to_cpu(ddf->phys->entries[pd].state);
		if ((st & (DDF_Online|DDF_Failed|DDF_Rebuilding)) ==
		    DDF_Online) {
			working++;
			avail[i] = 1;
		}
	}

	state = DDF_state_degraded;
	if (working == n_prim)
		state = DDF_state_optimal;
	else
		switch (vc->prl) {
		case DDF_RAID0:
		case DDF_CONCAT:
		case DDF_JBOD:
			state = DDF_state_failed;
			break;
		case DDF_RAID1:
			if (working == 0)
				state = DDF_state_failed;
			else if (working >= 2)
				state = DDF_state_part_optimal;
			break;
		case DDF_RAID1E:
			if (!enough(10, n_prim, array.layout, 1, avail))
				state = DDF_state_failed;
			break;
		case DDF_RAID4:
		case DDF_RAID5:
			if (working < n_prim - 1)
				state = DDF_state_failed;
			break;
		case DDF_RAID6:
			if (working < n_prim - 2)
				state = DDF_state_failed;
			else if (working == n_prim - 1)
				state = DDF_state_part_optimal;
			break;
		}
	return state;
}

static int secondary_state(int state, int other, int seclevel)
{
	if (state == DDF_state_optimal && other == DDF_state_optimal)
		return DDF_state_optimal;
	if (seclevel == DDF_2MIRRORED) {
		if (state == DDF_state_optimal || other == DDF_state_optimal)
			return DDF_state_part_optimal;
		if (state == DDF_state_failed && other == DDF_state_failed)
			return DDF_state_failed;
		return DDF_state_degraded;
	} else {
		if (state == DDF_state_failed || other == DDF_state_failed)
			return DDF_state_failed;
		if (state == DDF_state_degraded || other == DDF_state_degraded)
			return DDF_state_degraded;
		return DDF_state_part_optimal;
	}
}

static int get_svd_state(const struct ddf_super *ddf, const struct vcl *vcl)
{
	int state = get_bvd_state(ddf, &vcl->conf);
	unsigned int i;
	for (i = 1; i < vcl->conf.sec_elmnt_count; i++) {
		state = secondary_state(
			state,
			get_bvd_state(ddf, vcl->other_bvds[i-1]),
			vcl->conf.srl);
	}
	return state;
}

/*
 * The state of each disk is stored in the global phys_disk structure
 * in phys_disk.entries[n].state.
 * This makes various combinations awkward.
 * - When a device fails in any array, it must be failed in all arrays
 *   that include a part of this device.
 * - When a component is rebuilding, we cannot include it officially in the
 *   array unless this is the only array that uses the device.
 *
 * So: when transitioning:
 *   Online -> failed,  just set failed flag.  monitor will propagate
 *   spare -> online,   the device might need to be added to the array.
 *   spare -> failed,   just set failed.  Don't worry if in array or not.
 */
static void ddf_set_disk(struct active_array *a, int n, int state)
{
	struct ddf_super *ddf = a->container->sb;
	unsigned int inst = a->info.container_member, n_bvd;
	struct vcl *vcl;
	struct vd_config *vc = find_vdcr(ddf, inst, (unsigned int)n,
					 &n_bvd, &vcl);
	int pd;
	struct mdinfo *mdi;
	struct dl *dl;
	int update = 0;

	dprintf("%d to %x\n", n, state);
	if (vc == NULL) {
		dprintf("ddf: cannot find instance %d!!\n", inst);
		return;
	}
	/* Find the matching slot in 'info'. */
	for (mdi = a->info.devs; mdi; mdi = mdi->next)
		if (mdi->disk.raid_disk == n)
			break;
	if (!mdi) {
		pr_err("cannot find raid disk %d\n", n);
		return;
	}

	/* and find the 'dl' entry corresponding to that. */
	for (dl = ddf->dlist; dl; dl = dl->next)
		if (mdi->state_fd >= 0 &&
		    mdi->disk.major == dl->major &&
		    mdi->disk.minor == dl->minor)
			break;
	if (!dl) {
		pr_err("cannot find raid disk %d (%d/%d)\n",
		       n, mdi->disk.major, mdi->disk.minor);
		return;
	}

	pd = find_phys(ddf, vc->phys_refnum[n_bvd]);
	if (pd < 0 || pd != dl->pdnum) {
		/* disk doesn't currently exist or has changed.
		 * If it is now in_sync, insert it. */
		dprintf("phys disk not found for %d: %d/%d ref %08x\n",
			dl->pdnum, dl->major, dl->minor,
			be32_to_cpu(dl->disk.refnum));
		dprintf("array %u disk %u ref %08x pd %d\n",
			inst, n_bvd,
			be32_to_cpu(vc->phys_refnum[n_bvd]), pd);
		if ((state & DS_INSYNC) && ! (state & DS_FAULTY) &&
		    dl->pdnum >= 0) {
			pd = dl->pdnum;
			vc->phys_refnum[n_bvd] = dl->disk.refnum;
			LBA_OFFSET(ddf, vc)[n_bvd] =
				cpu_to_be64(mdi->data_offset);
			be16_clear(ddf->phys->entries[pd].type,
				   cpu_to_be16(DDF_Global_Spare));
			be16_set(ddf->phys->entries[pd].type,
				 cpu_to_be16(DDF_Active_in_VD));
			update = 1;
		}
	} else {
		be16 old = ddf->phys->entries[pd].state;
		if (state & DS_FAULTY)
			be16_set(ddf->phys->entries[pd].state,
				 cpu_to_be16(DDF_Failed));
		if (state & DS_INSYNC) {
			be16_set(ddf->phys->entries[pd].state,
				 cpu_to_be16(DDF_Online));
			be16_clear(ddf->phys->entries[pd].state,
				   cpu_to_be16(DDF_Rebuilding));
		}
		if (!be16_eq(old, ddf->phys->entries[pd].state))
			update = 1;
	}

	dprintf("ddf: set_disk %d (%08x) to %x->%02x\n", n,
		be32_to_cpu(dl->disk.refnum), state,
		be16_to_cpu(ddf->phys->entries[pd].state));

	/* Now we need to check the state of the array and update
	 * virtual_disk.entries[n].state.
	 * It needs to be one of "optimal", "degraded", "failed".
	 * I don't understand 'deleted' or 'missing'.
	 */
	state = get_svd_state(ddf, vcl);

	if (ddf->virt->entries[inst].state !=
	    ((ddf->virt->entries[inst].state & ~DDF_state_mask)
	     | state)) {
		ddf->virt->entries[inst].state =
			(ddf->virt->entries[inst].state & ~DDF_state_mask)
			| state;
		update = 1;
	}
	if (update)
		ddf_set_updates_pending(ddf, vc);
}

static void ddf_sync_metadata(struct supertype *st)
{
	/*
	 * Write all data to all devices.
	 * Later, we might be able to track whether only local changes
	 * have been made, or whether any global data has been changed,
	 * but ddf is sufficiently weird that it probably always
	 * changes global data ....
	 */
	struct ddf_super *ddf = st->sb;
	if (!ddf->updates_pending)
		return;
	ddf->updates_pending = 0;
	__write_init_super_ddf(st);
	dprintf("ddf: sync_metadata\n");
}

static int del_from_conflist(struct vcl **list, const char *guid)
{
	struct vcl **p;
	int found = 0;
	for (p = list; p && *p; p = &((*p)->next))
		if (!memcmp((*p)->conf.guid, guid, DDF_GUID_LEN)) {
			found = 1;
			*p = (*p)->next;
		}
	return found;
}

static int _kill_subarray_ddf(struct ddf_super *ddf, const char *guid)
{
	struct dl *dl;
	unsigned int vdnum, i;
	vdnum = find_vde_by_guid(ddf, guid);
	if (vdnum == DDF_NOTFOUND) {
		pr_err("could not find VD %s\n", guid_str(guid));
		return -1;
	}
	if (del_from_conflist(&ddf->conflist, guid) == 0) {
		pr_err("could not find conf %s\n", guid_str(guid));
		return -1;
	}
	for (dl = ddf->dlist; dl; dl = dl->next)
		for (i = 0; i < ddf->max_part; i++)
			if (dl->vlist[i] != NULL &&
			    !memcmp(dl->vlist[i]->conf.guid, guid,
				    DDF_GUID_LEN))
				dl->vlist[i] = NULL;
	memset(ddf->virt->entries[vdnum].guid, 0xff, DDF_GUID_LEN);
	dprintf("deleted %s\n", guid_str(guid));
	return 0;
}

static int kill_subarray_ddf(struct supertype *st)
{
	struct ddf_super *ddf = st->sb;
	/*
	 *  currentconf is set in container_content_ddf,
	 *  called with subarray arg
	 */
	struct vcl *victim = ddf->currentconf;
	struct vd_config *conf;
	unsigned int vdnum;

	ddf->currentconf = NULL;
	if (!victim) {
		pr_err("nothing to kill\n");
		return -1;
	}
	conf = &victim->conf;
	vdnum = find_vde_by_guid(ddf, conf->guid);
	if (vdnum == DDF_NOTFOUND) {
		pr_err("could not find VD %s\n", guid_str(conf->guid));
		return -1;
	}
	if (st->update_tail) {
		struct virtual_disk *vd;
		int len = sizeof(struct virtual_disk)
			+ sizeof(struct virtual_entry);
		vd = xmalloc(len);
		if (vd == NULL) {
			pr_err("failed to allocate %d bytes\n", len);
			return -1;
		}
		memset(vd, 0 , len);
		vd->magic = DDF_VIRT_RECORDS_MAGIC;
		vd->populated_vdes = cpu_to_be16(0);
		memcpy(vd->entries[0].guid, conf->guid, DDF_GUID_LEN);
		/* we use DDF_state_deleted as marker */
		vd->entries[0].state = DDF_state_deleted;
		append_metadata_update(st, vd, len);
	} else {
		_kill_subarray_ddf(ddf, conf->guid);
		ddf_set_updates_pending(ddf, NULL);
		ddf_sync_metadata(st);
	}
	return 0;
}

static void copy_matching_bvd(struct ddf_super *ddf,
			      struct vd_config *conf,
			      const struct metadata_update *update)
{
	unsigned int mppe =
		be16_to_cpu(ddf->anchor.max_primary_element_entries);
	unsigned int len = ddf->conf_rec_len * 512;
	char *p;
	struct vd_config *vc;
	for (p = update->buf; p < update->buf + update->len; p += len) {
		vc = (struct vd_config *) p;
		if (vc->sec_elmnt_seq == conf->sec_elmnt_seq) {
			memcpy(conf->phys_refnum, vc->phys_refnum,
			       mppe * (sizeof(__u32) + sizeof(__u64)));
			return;
		}
	}
	pr_err("no match for BVD %d of %s in update\n",
	       conf->sec_elmnt_seq, guid_str(conf->guid));
}

static void ddf_process_phys_update(struct supertype *st,
				    struct metadata_update *update)
{
	struct ddf_super *ddf = st->sb;
	struct phys_disk *pd;
	unsigned int ent;

	pd = (struct phys_disk*)update->buf;
	ent = be16_to_cpu(pd->used_pdes);
	if (ent >= be16_to_cpu(ddf->phys->max_pdes))
		return;
	if (be16_and(pd->entries[0].state, cpu_to_be16(DDF_Missing))) {
		struct dl **dlp;
		/* removing this disk. */
		be16_set(ddf->phys->entries[ent].state,
			 cpu_to_be16(DDF_Missing));
		for (dlp = &ddf->dlist; *dlp; dlp = &(*dlp)->next) {
			struct dl *dl = *dlp;
			if (dl->pdnum == (signed)ent) {
				close(dl->fd);
				dl->fd = -1;
				*dlp = dl->next;
				update->space = dl->devname;
				*(void**)dl = update->space_list;
				update->space_list = (void**)dl;
				break;
			}
		}
		ddf_set_updates_pending(ddf, NULL);
		return;
	}
	if (!all_ff(ddf->phys->entries[ent].guid))
		return;
	ddf->phys->entries[ent] = pd->entries[0];
	ddf->phys->used_pdes = cpu_to_be16
		(1 + be16_to_cpu(ddf->phys->used_pdes));
	ddf_set_updates_pending(ddf, NULL);
	if (ddf->add_list) {
		struct active_array *a;
		struct dl *al = ddf->add_list;
		ddf->add_list = al->next;

		al->next = ddf->dlist;
		ddf->dlist = al;

		/* As a device has been added, we should check
		 * for any degraded devices that might make
		 * use of this spare */
		for (a = st->arrays ; a; a=a->next)
			a->check_degraded = 1;
	}
}

static void ddf_process_virt_update(struct supertype *st,
				    struct metadata_update *update)
{
	struct ddf_super *ddf = st->sb;
	struct virtual_disk *vd;
	unsigned int ent;

	vd = (struct virtual_disk*)update->buf;

	if (vd->entries[0].state == DDF_state_deleted) {
		if (_kill_subarray_ddf(ddf, vd->entries[0].guid))
			return;
	} else {
		ent = find_vde_by_guid(ddf, vd->entries[0].guid);
		if (ent != DDF_NOTFOUND) {
			dprintf("VD %s exists already in slot %d\n",
				guid_str(vd->entries[0].guid),
				ent);
			return;
		}
		ent = find_unused_vde(ddf);
		if (ent == DDF_NOTFOUND)
			return;
		ddf->virt->entries[ent] = vd->entries[0];
		ddf->virt->populated_vdes =
			cpu_to_be16(
				1 + be16_to_cpu(
					ddf->virt->populated_vdes));
		dprintf("added VD %s in slot %d(s=%02x i=%02x)\n",
			guid_str(vd->entries[0].guid), ent,
			ddf->virt->entries[ent].state,
			ddf->virt->entries[ent].init_state);
	}
	ddf_set_updates_pending(ddf, NULL);
}

static void ddf_remove_failed(struct ddf_super *ddf)
{
	/* Now remove any 'Failed' devices that are not part
	 * of any VD.  They will have the Transition flag set.
	 * Once done, we need to update all dl->pdnum numbers.
	 */
	unsigned int pdnum;
	unsigned int pd2 = 0;
	struct dl *dl;

	for (pdnum = 0; pdnum < be16_to_cpu(ddf->phys->max_pdes);
	     pdnum++) {
		if (be32_to_cpu(ddf->phys->entries[pdnum].refnum) ==
		    0xFFFFFFFF)
			continue;
		if (be16_and(ddf->phys->entries[pdnum].state,
			     cpu_to_be16(DDF_Failed)) &&
		    be16_and(ddf->phys->entries[pdnum].state,
			     cpu_to_be16(DDF_Transition))) {
			/* skip this one unless in dlist*/
			for (dl = ddf->dlist; dl; dl = dl->next)
				if (dl->pdnum == (int)pdnum)
					break;
			if (!dl)
				continue;
		}
		if (pdnum == pd2)
			pd2++;
		else {
			ddf->phys->entries[pd2] =
				ddf->phys->entries[pdnum];
			for (dl = ddf->dlist; dl; dl = dl->next)
				if (dl->pdnum == (int)pdnum)
					dl->pdnum = pd2;
			pd2++;
		}
	}
	ddf->phys->used_pdes = cpu_to_be16(pd2);
	while (pd2 < pdnum) {
		memset(ddf->phys->entries[pd2].guid, 0xff,
		       DDF_GUID_LEN);
		pd2++;
	}
}

static void ddf_update_vlist(struct ddf_super *ddf, struct dl *dl)
{
	struct vcl *vcl;
	unsigned int vn = 0;
	int in_degraded = 0;

	if (dl->pdnum < 0)
		return;
	for (vcl = ddf->conflist; vcl ; vcl = vcl->next) {
		unsigned int dn, ibvd;
		const struct vd_config *conf;
		int vstate;
		dn = get_pd_index_from_refnum(vcl,
					      dl->disk.refnum,
					      ddf->mppe,
					      &conf, &ibvd);
		if (dn == DDF_NOTFOUND)
			continue;
		dprintf("dev %d/%08x has %s (sec=%u) at %d\n",
			dl->pdnum,
			be32_to_cpu(dl->disk.refnum),
			guid_str(conf->guid),
			conf->sec_elmnt_seq, vn);
		/* Clear the Transition flag */
		if (be16_and
		    (ddf->phys->entries[dl->pdnum].state,
		     cpu_to_be16(DDF_Failed)))
			be16_clear(ddf->phys
				   ->entries[dl->pdnum].state,
				   cpu_to_be16(DDF_Transition));
		dl->vlist[vn++] = vcl;
		vstate = ddf->virt->entries[vcl->vcnum].state
			& DDF_state_mask;
		if (vstate == DDF_state_degraded ||
		    vstate == DDF_state_part_optimal)
			in_degraded = 1;
	}
	while (vn < ddf->max_part)
		dl->vlist[vn++] = NULL;
	if (dl->vlist[0]) {
		be16_clear(ddf->phys->entries[dl->pdnum].type,
			   cpu_to_be16(DDF_Global_Spare));
		if (!be16_and(ddf->phys
			      ->entries[dl->pdnum].type,
			      cpu_to_be16(DDF_Active_in_VD))) {
			be16_set(ddf->phys
				 ->entries[dl->pdnum].type,
				 cpu_to_be16(DDF_Active_in_VD));
			if (in_degraded)
				be16_set(ddf->phys
					 ->entries[dl->pdnum]
					 .state,
					 cpu_to_be16
					 (DDF_Rebuilding));
		}
	}
	if (dl->spare) {
		be16_clear(ddf->phys->entries[dl->pdnum].type,
			   cpu_to_be16(DDF_Global_Spare));
		be16_set(ddf->phys->entries[dl->pdnum].type,
			 cpu_to_be16(DDF_Spare));
	}
	if (!dl->vlist[0] && !dl->spare) {
		be16_set(ddf->phys->entries[dl->pdnum].type,
			 cpu_to_be16(DDF_Global_Spare));
		be16_clear(ddf->phys->entries[dl->pdnum].type,
			   cpu_to_be16(DDF_Spare));
		be16_clear(ddf->phys->entries[dl->pdnum].type,
			   cpu_to_be16(DDF_Active_in_VD));
	}
}

static void ddf_process_conf_update(struct supertype *st,
				    struct metadata_update *update)
{
	struct ddf_super *ddf = st->sb;
	struct vd_config *vc;
	struct vcl *vcl;
	struct dl *dl;
	unsigned int ent;
	unsigned int pdnum, len;

	vc = (struct vd_config*)update->buf;
	len = ddf->conf_rec_len * 512;
	if ((unsigned int)update->len != len * vc->sec_elmnt_count) {
		pr_err("%s: insufficient data (%d) for %u BVDs\n",
		       guid_str(vc->guid), update->len,
		       vc->sec_elmnt_count);
		return;
	}
	for (vcl = ddf->conflist; vcl ; vcl = vcl->next)
		if (memcmp(vcl->conf.guid, vc->guid, DDF_GUID_LEN) == 0)
			break;
	dprintf("conf update for %s (%s)\n",
		guid_str(vc->guid), (vcl ? "old" : "new"));
	if (vcl) {
		/* An update, just copy the phys_refnum and lba_offset
		 * fields
		 */
		unsigned int i;
		unsigned int k;
		copy_matching_bvd(ddf, &vcl->conf, update);
		for (k = 0; k < be16_to_cpu(vc->prim_elmnt_count); k++)
			dprintf("BVD %u has %08x at %llu\n", 0,
				be32_to_cpu(vcl->conf.phys_refnum[k]),
				be64_to_cpu(LBA_OFFSET(ddf,
						       &vcl->conf)[k]));
		for (i = 1; i < vc->sec_elmnt_count; i++) {
			copy_matching_bvd(ddf, vcl->other_bvds[i-1],
					  update);
			for (k = 0; k < be16_to_cpu(
				     vc->prim_elmnt_count); k++)
				dprintf("BVD %u has %08x at %llu\n", i,
					be32_to_cpu
					(vcl->other_bvds[i-1]->
					 phys_refnum[k]),
					be64_to_cpu
					(LBA_OFFSET
					 (ddf,
					  vcl->other_bvds[i-1])[k]));
		}
	} else {
		/* A new VD_CONF */
		unsigned int i;
		if (!update->space)
			return;
		vcl = update->space;
		update->space = NULL;
		vcl->next = ddf->conflist;
		memcpy(&vcl->conf, vc, len);
		ent = find_vde_by_guid(ddf, vc->guid);
		if (ent == DDF_NOTFOUND)
			return;
		vcl->vcnum = ent;
		ddf->conflist = vcl;
		for (i = 1; i < vc->sec_elmnt_count; i++)
			memcpy(vcl->other_bvds[i-1],
			       update->buf + len * i, len);
	}
	/* Set DDF_Transition on all Failed devices - to help
	 * us detect those that are no longer in use
	 */
	for (pdnum = 0; pdnum < be16_to_cpu(ddf->phys->max_pdes);
	     pdnum++)
		if (be16_and(ddf->phys->entries[pdnum].state,
			     cpu_to_be16(DDF_Failed)))
			be16_set(ddf->phys->entries[pdnum].state,
				 cpu_to_be16(DDF_Transition));

	/* Now make sure vlist is correct for each dl. */
	for (dl = ddf->dlist; dl; dl = dl->next)
		ddf_update_vlist(ddf, dl);
	ddf_remove_failed(ddf);

	ddf_set_updates_pending(ddf, vc);
}

static void ddf_process_update(struct supertype *st,
			       struct metadata_update *update)
{
	/* Apply this update to the metadata.
	 * The first 4 bytes are a DDF_*_MAGIC which guides
	 * our actions.
	 * Possible update are:
	 *  DDF_PHYS_RECORDS_MAGIC
	 *    Add a new physical device or remove an old one.
	 *    Changes to this record only happen implicitly.
	 *    used_pdes is the device number.
	 *  DDF_VIRT_RECORDS_MAGIC
	 *    Add a new VD.  Possibly also change the 'access' bits.
	 *    populated_vdes is the entry number.
	 *  DDF_VD_CONF_MAGIC
	 *    New or updated VD.  the VIRT_RECORD must already
	 *    exist.  For an update, phys_refnum and lba_offset
	 *    (at least) are updated, and the VD_CONF must
	 *    be written to precisely those devices listed with
	 *    a phys_refnum.
	 *  DDF_SPARE_ASSIGN_MAGIC
	 *    replacement Spare Assignment Record... but for which device?
	 *
	 * So, e.g.:
	 *  - to create a new array, we send a VIRT_RECORD and
	 *    a VD_CONF.  Then assemble and start the array.
	 *  - to activate a spare we send a VD_CONF to add the phys_refnum
	 *    and offset.  This will also mark the spare as active with
	 *    a spare-assignment record.
	 */
	be32 *magic = (be32 *)update->buf;

	dprintf("Process update %x\n", be32_to_cpu(*magic));

	if (be32_eq(*magic, DDF_PHYS_RECORDS_MAGIC)) {
		if (update->len == (sizeof(struct phys_disk) +
				    sizeof(struct phys_disk_entry)))
			ddf_process_phys_update(st, update);
	} else if (be32_eq(*magic, DDF_VIRT_RECORDS_MAGIC)) {
		if (update->len == (sizeof(struct virtual_disk) +
				    sizeof(struct virtual_entry)))
			ddf_process_virt_update(st, update);
	} else if (be32_eq(*magic, DDF_VD_CONF_MAGIC)) {
		ddf_process_conf_update(st, update);
	}
	/* case DDF_SPARE_ASSIGN_MAGIC */
}

static int ddf_prepare_update(struct supertype *st,
			      struct metadata_update *update)
{
	/* This update arrived at managemon.
	 * We are about to pass it to monitor.
	 * If a malloc is needed, do it here.
	 */
	struct ddf_super *ddf = st->sb;
	be32 *magic;
	if (update->len < 4)
		return 0;
	magic = (be32 *)update->buf;
	if (be32_eq(*magic, DDF_VD_CONF_MAGIC)) {
		struct vcl *vcl;
		struct vd_config *conf;
		if (update->len < (int)sizeof(*conf))
			return 0;
		conf = (struct vd_config *) update->buf;
		if (posix_memalign(&update->space, 512,
				   offsetof(struct vcl, conf)
				   + ddf->conf_rec_len * 512) != 0) {
			update->space = NULL;
			return 0;
		}
		vcl = update->space;
		vcl->conf.sec_elmnt_count = conf->sec_elmnt_count;
		if (alloc_other_bvds(ddf, vcl) != 0) {
			free(update->space);
			update->space = NULL;
			return 0;
		}
	}
	return 1;
}

/*
 * Check degraded state of a RAID10.
 * returns 2 for good, 1 for degraded, 0 for failed, and -1 for error
 */
static int raid10_degraded(struct mdinfo *info)
{
	int n_prim, n_bvds;
	int i;
	struct mdinfo *d;
	char *found;
	int ret = -1;

	n_prim = info->array.layout & ~0x100;
	n_bvds = info->array.raid_disks / n_prim;
	found = xmalloc(n_bvds);
	if (found == NULL)
		return ret;
	memset(found, 0, n_bvds);
	for (d = info->devs; d; d = d->next) {
		i = d->disk.raid_disk / n_prim;
		if (i >= n_bvds) {
			pr_err("BUG: invalid raid disk\n");
			goto out;
		}
		if (d->state_fd > 0)
			found[i]++;
	}
	ret = 2;
	for (i = 0; i < n_bvds; i++)
		if (!found[i]) {
			dprintf("BVD %d/%d failed\n", i, n_bvds);
			ret = 0;
			goto out;
		} else if (found[i] < n_prim) {
			dprintf("BVD %d/%d degraded\n", i, n_bvds);
			ret = 1;
		}
out:
	free(found);
	return ret;
}

/*
 * Check if the array 'a' is degraded but not failed.
 * If it is, find as many spares as are available and needed and
 * arrange for their inclusion.
 * We only choose devices which are not already in the array,
 * and prefer those with a spare-assignment to this array.
 * Otherwise we choose global spares - assuming always that
 * there is enough room.
 * For each spare that we assign, we return an 'mdinfo' which
 * describes the position for the device in the array.
 * We also add to 'updates' a DDF_VD_CONF_MAGIC update with
 * the new phys_refnum and lba_offset values.
 *
 * Only worry about BVDs at the moment.
 */
static struct mdinfo *ddf_activate_spare(struct active_array *a,
					 struct metadata_update **updates)
{
	int working = 0;
	struct mdinfo *d;
	struct ddf_super *ddf = a->container->sb;
	int global_ok = 0;
	struct mdinfo *rv = NULL;
	struct mdinfo *di;
	struct metadata_update *mu;
	struct dl *dl;
	int i;
	unsigned int j;
	struct vcl *vcl;
	struct vd_config *vc;
	unsigned int n_bvd;

	for (d = a->info.devs ; d ; d = d->next) {
		if ((d->curr_state & DS_FAULTY) &&
		    d->state_fd >= 0)
			/* wait for Removal to happen */
			return NULL;
		if (d->state_fd >= 0)
			working ++;
	}

	dprintf("working=%d (%d) level=%d\n", working,
		a->info.array.raid_disks,
		a->info.array.level);
	if (working == a->info.array.raid_disks)
		return NULL; /* array not degraded */
	switch (a->info.array.level) {
	case 1:
		if (working == 0)
			return NULL; /* failed */
		break;
	case 4:
	case 5:
		if (working < a->info.array.raid_disks - 1)
			return NULL; /* failed */
		break;
	case 6:
		if (working < a->info.array.raid_disks - 2)
			return NULL; /* failed */
		break;
	case 10:
		if (raid10_degraded(&a->info) < 1)
			return NULL;
		break;
	default: /* concat or stripe */
		return NULL; /* failed */
	}

	/* For each slot, if it is not working, find a spare */
	dl = ddf->dlist;
	for (i = 0; i < a->info.array.raid_disks; i++) {
		for (d = a->info.devs ; d ; d = d->next)
			if (d->disk.raid_disk == i)
				break;
		dprintf("found %d: %p %x\n", i, d, d?d->curr_state:0);
		if (d && (d->state_fd >= 0))
			continue;

		/* OK, this device needs recovery.  Find a spare */
	again:
		for ( ; dl ; dl = dl->next) {
			unsigned long long esize;
			unsigned long long pos;
			struct mdinfo *d2;
			int is_global = 0;
			int is_dedicated = 0;
			be16 state;

			if (dl->pdnum < 0)
				continue;
			state = ddf->phys->entries[dl->pdnum].state;
			if (be16_and(state,
				     cpu_to_be16(DDF_Failed|DDF_Missing)) ||
			    !be16_and(state,
				      cpu_to_be16(DDF_Online)))
				continue;

			/* If in this array, skip */
			for (d2 = a->info.devs ; d2 ; d2 = d2->next)
				if (d2->state_fd >= 0 &&
				    d2->disk.major == dl->major &&
				    d2->disk.minor == dl->minor) {
					dprintf("%x:%x (%08x) already in array\n",
						dl->major, dl->minor,
						be32_to_cpu(dl->disk.refnum));
					break;
				}
			if (d2)
				continue;
			if (be16_and(ddf->phys->entries[dl->pdnum].type,
				     cpu_to_be16(DDF_Spare))) {
				/* Check spare assign record */
				if (dl->spare) {
					if (dl->spare->type & DDF_spare_dedicated) {
						/* check spare_ents for guid */
						unsigned int j;
						for (j = 0 ;
						     j < be16_to_cpu
							     (dl->spare
							      ->populated);
						     j++) {
							if (memcmp(dl->spare->spare_ents[j].guid,
								   ddf->virt->entries[a->info.container_member].guid,
								   DDF_GUID_LEN) == 0)
								is_dedicated = 1;
						}
					} else
						is_global = 1;
				}
			} else if (be16_and(ddf->phys->entries[dl->pdnum].type,
					    cpu_to_be16(DDF_Global_Spare))) {
				is_global = 1;
			} else if (!be16_and(ddf->phys
					     ->entries[dl->pdnum].state,
					     cpu_to_be16(DDF_Failed))) {
				/* we can possibly use some of this */
				is_global = 1;
			}
			if ( ! (is_dedicated ||
				(is_global && global_ok))) {
				dprintf("%x:%x not suitable: %d %d\n", dl->major, dl->minor,
					is_dedicated, is_global);
				continue;
			}

			/* We are allowed to use this device - is there space?
			 * We need a->info.component_size sectors */
			esize = a->info.component_size;
			pos = find_space(ddf, dl, INVALID_SECTORS, &esize);

			if (esize < a->info.component_size) {
				dprintf("%x:%x has no room: %llu %llu\n",
					dl->major, dl->minor,
					esize, a->info.component_size);
				/* No room */
				continue;
			}

			/* Cool, we have a device with some space at pos */
			di = xcalloc(1, sizeof(*di));
			di->disk.number = i;
			di->disk.raid_disk = i;
			di->disk.major = dl->major;
			di->disk.minor = dl->minor;
			di->disk.state = 0;
			di->recovery_start = 0;
			di->data_offset = pos;
			di->component_size = a->info.component_size;
			di->next = rv;
			rv = di;
			dprintf("%x:%x (%08x) to be %d at %llu\n",
				dl->major, dl->minor,
				be32_to_cpu(dl->disk.refnum), i, pos);

			break;
		}
		if (!dl && ! global_ok) {
			/* not enough dedicated spares, try global */
			global_ok = 1;
			dl = ddf->dlist;
			goto again;
		}
	}

	if (!rv)
		/* No spares found */
		return rv;
	/* Now 'rv' has a list of devices to return.
	 * Create a metadata_update record to update the
	 * phys_refnum and lba_offset values
	 */
	vc = find_vdcr(ddf, a->info.container_member, rv->disk.raid_disk,
		       &n_bvd, &vcl);
	if (vc == NULL)
		return NULL;

	mu = xmalloc(sizeof(*mu));
	if (posix_memalign(&mu->space, 512, sizeof(struct vcl)) != 0) {
		free(mu);
		mu = NULL;
	}

	mu->len = ddf->conf_rec_len * 512 * vcl->conf.sec_elmnt_count;
	mu->buf = xmalloc(mu->len);
	mu->space = NULL;
	mu->space_list = NULL;
	mu->next = *updates;
	memcpy(mu->buf, &vcl->conf, ddf->conf_rec_len * 512);
	for (j = 1; j < vcl->conf.sec_elmnt_count; j++)
		memcpy(mu->buf + j * ddf->conf_rec_len * 512,
		       vcl->other_bvds[j-1], ddf->conf_rec_len * 512);

	vc = (struct vd_config*)mu->buf;
	for (di = rv ; di ; di = di->next) {
		unsigned int i_sec, i_prim;
		i_sec = di->disk.raid_disk
			/ be16_to_cpu(vcl->conf.prim_elmnt_count);
		i_prim = di->disk.raid_disk
			% be16_to_cpu(vcl->conf.prim_elmnt_count);
		vc = (struct vd_config *)(mu->buf
					  + i_sec * ddf->conf_rec_len * 512);
		for (dl = ddf->dlist; dl; dl = dl->next)
			if (dl->major == di->disk.major &&
			    dl->minor == di->disk.minor)
				break;
		if (!dl || dl->pdnum < 0) {
			pr_err("BUG: can't find disk %d (%d/%d)\n",
			       di->disk.raid_disk,
			       di->disk.major, di->disk.minor);
			return NULL;
		}
		vc->phys_refnum[i_prim] = ddf->phys->entries[dl->pdnum].refnum;
		LBA_OFFSET(ddf, vc)[i_prim] = cpu_to_be64(di->data_offset);
		dprintf("BVD %u gets %u: %08x at %llu\n", i_sec, i_prim,
			be32_to_cpu(vc->phys_refnum[i_prim]),
			be64_to_cpu(LBA_OFFSET(ddf, vc)[i_prim]));
	}
	*updates = mu;
	return rv;
}

static int ddf_level_to_layout(int level)
{
	switch(level) {
	case 0:
	case 1:
		return 0;
	case 5:
		return ALGORITHM_LEFT_SYMMETRIC;
	case 6:
		return ALGORITHM_ROTATING_N_CONTINUE;
	case 10:
		return 0x102;
	default:
		return UnSet;
	}
}

static void default_geometry_ddf(struct supertype *st, int *level, int *layout, int *chunk)
{
	if (level && *level == UnSet)
		*level = LEVEL_CONTAINER;

	if (level && layout && *layout == UnSet)
		*layout = ddf_level_to_layout(*level);
}

struct superswitch super_ddf = {
	.examine_super	= examine_super_ddf,
	.brief_examine_super = brief_examine_super_ddf,
	.brief_examine_subarrays = brief_examine_subarrays_ddf,
	.export_examine_super = export_examine_super_ddf,
	.detail_super	= detail_super_ddf,
	.brief_detail_super = brief_detail_super_ddf,
	.validate_geometry = validate_geometry_ddf,
	.write_init_super = write_init_super_ddf,
	.add_to_super	= add_to_super_ddf,
	.remove_from_super = remove_from_super_ddf,
	.load_container	= load_container_ddf,
	.copy_metadata = copy_metadata_ddf,
	.kill_subarray  = kill_subarray_ddf,
	.match_home	= match_home_ddf,
	.uuid_from_super= uuid_from_super_ddf,
	.getinfo_super  = getinfo_super_ddf,
	.update_super	= update_super_ddf,

	.avail_size	= avail_size_ddf,

	.compare_super	= compare_super_ddf,

	.load_super	= load_super_ddf,
	.init_super	= init_super_ddf,
	.store_super	= store_super_ddf,
	.free_super	= free_super_ddf,
	.match_metadata_desc = match_metadata_desc_ddf,
	.container_content = container_content_ddf,
	.default_geometry = default_geometry_ddf,

	.external	= 1,

/* for mdmon */
	.open_new       = ddf_open_new,
	.set_array_state= ddf_set_array_state,
	.set_disk       = ddf_set_disk,
	.sync_metadata  = ddf_sync_metadata,
	.process_update	= ddf_process_update,
	.prepare_update	= ddf_prepare_update,
	.activate_spare = ddf_activate_spare,
	.name = "ddf",
};
