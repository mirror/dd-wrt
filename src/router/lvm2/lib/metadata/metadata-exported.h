/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2018 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * This is the representation of LVM metadata that is being adapted
 * for library export.
 */

#ifndef _LVM_METADATA_EXPORTED_H
#define _LVM_METADATA_EXPORTED_H

#include "lib/uuid/uuid.h"
#include "lib/metadata/pv.h"
#include "lib/metadata/vg.h"
#include "lib/metadata/lv.h"
#include "lib/misc/lvm-percent.h"

#define MAX_STRIPES 128U
#define SECTOR_SHIFT 9L
#define SECTOR_SIZE ( 1L << SECTOR_SHIFT )
#define STRIPE_SIZE_MIN ( (unsigned) lvm_getpagesize() >> SECTOR_SHIFT)	/* PAGESIZE in sectors */
#define STRIPE_SIZE_MAX ( 512L * 1024L >> SECTOR_SHIFT)	/* 512 KB in sectors */
#define STRIPE_SIZE_LIMIT ((UINT_MAX >> 2) + 1)
#define MAX_RESTRICTED_LVS 255	/* Used by FMT_RESTRICTED_LVIDS */
#define MAX_EXTENT_SIZE ((uint32_t) -1)
#define MIN_NON_POWER2_EXTENT_SIZE (128U * 2U)	/* 128KB in sectors */

#define HISTORICAL_LV_PREFIX "-"

/* Layer suffix */
#define MIRROR_SYNC_LAYER "_mimagetmp"

/* PV extension flags */
#define PV_EXT_USED		UINT32_C(0x00000001)

/* Various flags */
/* Note that the bits no longer necessarily correspond to LVM1 disk format */

#define PARTIAL_VG		UINT64_C(0x0000000000000001)	/* VG */
#define EXPORTED_VG		UINT64_C(0x0000000000000002)	/* VG PV */
#define RESIZEABLE_VG		UINT64_C(0x0000000000000004)	/* VG */

/* May any free extents on this PV be used or must they be left free? */
#define ALLOCATABLE_PV		UINT64_C(0x0000000000000008)	/* PV */
#define ARCHIVED_VG		ALLOCATABLE_PV		/* VG, reuse same bit */

//#define SPINDOWN_LV		UINT64_C(0x0000000000000010)	/* LV */
//#define BADBLOCK_ON		UINT64_C(0x0000000000000020)	/* LV */
#define VISIBLE_LV		UINT64_C(0x0000000000000040)	/* LV */
#define FIXED_MINOR		UINT64_C(0x0000000000000080)	/* LV */

#define LVM_READ		UINT64_C(0x0000000000000100)	/* LV, VG */
#define LVM_WRITE		UINT64_C(0x0000000000000200)	/* LV, VG */
#define LVM_WRITE_LOCKED	UINT64_C(0x0020000000000000)    /* LV, VG */

#define CLUSTERED		UINT64_C(0x0000000000000400)	/* VG */
//#define SHARED		UINT64_C(0x0000000000000800)	/* VG */

/* FIXME Remove when metadata restructuring is completed */
#define SNAPSHOT		UINT64_C(0x0000000000001000)	/* LV - internal use only */
#define PVMOVE			UINT64_C(0x0000000000002000)	/* VG LV SEG */
#define LOCKED			UINT64_C(0x0000000000004000)	/* LV */
#define MIRRORED		UINT64_C(0x0000000000008000)	/* LV - internal use only */
#define VIRTUAL			UINT64_C(0x0000000000010000)	/* LV - internal use only */
#define MIRROR			UINT64_C(0x0002000000000000)    /* LV - Internal use only */
#define MIRROR_LOG		UINT64_C(0x0000000000020000)	/* LV - Internal use only */
#define MIRROR_IMAGE		UINT64_C(0x0000000000040000)	/* LV - Internal use only */

#define LV_NOTSYNCED		UINT64_C(0x0000000000080000)	/* LV */
#define LV_REBUILD		UINT64_C(0x0000000000100000)	/* LV */
//#define PRECOMMITTED		UINT64_C(0x0000000000200000)	/* VG - internal use only */
#define CONVERTING		UINT64_C(0x0000000000400000)	/* LV */

#define MISSING_PV		UINT64_C(0x0000000000800000)	/* PV */
#define PV_MOVED_VG		UINT64_C(0x4000000000000000)	/* PV - Moved to a new VG */
#define PARTIAL_LV		UINT64_C(0x0000000001000000)	/* LV - derived flag, not
							   written out in metadata*/

//#define POSTORDER_FLAG	UINT64_C(0x0000000002000000) /* Not real flags, reserved for
//#define POSTORDER_OPEN_FLAG	UINT64_C(0x0000000004000000)    temporary use inside vg_read_internal. */
#define VIRTUAL_ORIGIN		UINT64_C(0x0000000008000000)	/* LV - internal use only */

#define MERGING			UINT64_C(0x0000000010000000)	/* LV SEG */

#define UNLABELLED_PV		UINT64_C(0x0000000080000000)	/* PV -this PV had no label written yet */

#define RAID			UINT64_C(0x0000000100000000)	/* LV - Internal use only */
#define RAID_META		UINT64_C(0x0000000200000000)	/* LV - Internal use only */
#define RAID_IMAGE		UINT64_C(0x0000000400000000)	/* LV - Internal use only */

#define THIN_VOLUME		UINT64_C(0x0000001000000000)	/* LV - Internal use only */
#define THIN_POOL		UINT64_C(0x0000002000000000)	/* LV - Internal use only */
#define THIN_POOL_DATA		UINT64_C(0x0000004000000000)	/* LV - Internal use only */
#define THIN_POOL_METADATA	UINT64_C(0x0000008000000000)	/* LV - Internal use only */
#define POOL_METADATA_SPARE	UINT64_C(0x0000010000000000)	/* LV - Internal use only */
#define LV_WRITEMOSTLY		UINT64_C(0x0000020000000000)	/* LV (RAID1) */

#define LV_ACTIVATION_SKIP	UINT64_C(0x0000040000000000)	/* LV */
#define LV_NOSCAN		UINT64_C(0x0000080000000000)	/* LV - internal use only - the LV
									should not be scanned */
#define LV_TEMPORARY		UINT64_C(0x0000100000000000)	/* LV - internal use only - the LV
									is supposed to be created and
									removed or reactivated with
									this flag dropped during single
									LVM command execution. */

#define CACHE_POOL		UINT64_C(0x0000200000000000)    /* LV - Internal use only */
#define CACHE_POOL_DATA		UINT64_C(0x0000400000000000)    /* LV - Internal use only */
#define CACHE_POOL_METADATA	UINT64_C(0x0000800000000000)    /* LV - Internal use only */
#define CACHE			UINT64_C(0x0001000000000000)    /* LV - Internal use only */

#define LV_PENDING_DELETE	UINT64_C(0x0004000000000000)    /* LV - Internal use only */
#define LV_REMOVED		UINT64_C(0x0040000000000000)	/* LV - Internal use only
								   This flag is used to mark an LV once it has
								   been removed from the VG. It might still
								   be referenced on internal lists of LVs.
								   Any remaining references should check for
								   this flag and ignore the LV is set.
								   FIXME: Remove this flag once we have indexed
									  vg->removed_lvs for quick lookup.
								*/
#define LV_ERROR_WHEN_FULL	UINT64_C(0x0008000000000000)    /* LV - error when full */
#define PV_ALLOCATION_PROHIBITED	UINT64_C(0x0010000000000000)	/* PV - internal use only - allocation prohibited
									e.g. to prohibit allocation of a RAID image
									on a PV already holing an image of the RAID set */
#define LOCKD_SANLOCK_LV	UINT64_C(0x0080000000000000)	/* LV - Internal use only */
#define LV_RESHAPE_DELTA_DISKS_PLUS		UINT64_C(0x0100000000000000)    /* LV reshape flag delta disks plus image(s) */
#define LV_RESHAPE_DELTA_DISKS_MINUS		UINT64_C(0x0200000000000000)    /* LV reshape flag delta disks minus image(s) */

#define LV_REMOVE_AFTER_RESHAPE	UINT64_C(0x0400000000000000)	/* LV needs to be removed after a shrinking reshape */
#define LV_METADATA_FORMAT	UINT64_C(0x0800000000000000)    /* LV has segments with metadata format */

#define LV_RESHAPE		UINT64_C(0x1000000000000000)    /* Ongoing reshape (number of stripes, stripesize or raid algorithm change):
								   used as SEGTYPE_FLAG to prevent activation on old runtime */
#define LV_RESHAPE_DATA_OFFSET	UINT64_C(0x2000000000000000)    /* LV reshape flag data offset (out of place reshaping) */


#define LV_VDO			UINT64_C(0x0000000020000000)    /* LV - Internal user only */
#define LV_VDO_POOL		UINT64_C(0x0000000040000000)    /* LV - Internal user only */
#define LV_VDO_POOL_DATA	UINT64_C(0x8000000000000000)    /* LV - Internal user only */


/* Format features flags */
#define FMT_SEGMENTS		0x00000001U	/* Arbitrary segment params? */
// #define FMT_MDAS		0x00000002U	/* Proper metadata areas? */
#define FMT_TAGS		0x00000004U	/* Tagging? */
#define FMT_UNLIMITED_VOLS	0x00000008U	/* Unlimited PVs/LVs? */
#define FMT_RESTRICTED_LVIDS	0x00000010U	/* LVID <= 255 */
#define FMT_ORPHAN_ALLOCATABLE	0x00000020U	/* Orphan PV allocatable? */
//#define FMT_PRECOMMIT		0x00000040U	/* Supports pre-commit? */
#define FMT_RESIZE_PV		0x00000080U	/* Supports pvresize? */
#define FMT_UNLIMITED_STRIPESIZE 0x00000100U	/* Unlimited stripe size? */
#define FMT_RESTRICTED_READAHEAD 0x00000200U	/* Readahead restricted to 2-120? */
// #define FMT_BAS			0x000000400U	/* Supports bootloader areas? */
#define FMT_CONFIG_PROFILE	0x000000800U	/* Supports configuration profiles? */
// #define FMT_OBSOLETE		0x000001000U	/* Obsolete format? */
#define FMT_NON_POWER2_EXTENTS	0x000002000U	/* Non-power-of-2 extent sizes? */
// #define FMT_SYSTEMID_ON_PVS	0x000004000U	/* System ID is stored on PVs not VG */
#define FMT_PV_FLAGS		0x000008000U	/* Supports PV flags */

/* Mirror conversion type flags */
#define MIRROR_BY_SEG		0x00000001U	/* segment-by-segment mirror */
#define MIRROR_BY_LV		0x00000002U	/* mirror using whole mimage LVs */
#define MIRROR_BY_SEGMENTED_LV	0x00000004U	/* mirror using whole mimage LVs that
						 * preserve the segment structure */
#define MIRROR_SKIP_INIT_SYNC	0x00000010U	/* skip initial sync */

/* vg_read and vg_read_for_update flags */
#define READ_ALLOW_INCONSISTENT	0x00010000U
#define READ_ALLOW_EXPORTED	0x00020000U
#define READ_OK_NOTFOUND	0x00040000U
#define READ_WARN_INCONSISTENT	0x00080000U
#define READ_FOR_UPDATE		0x00100000U /* A meta-flag, useful with toollib for_each_* functions. */
#define PROCESS_SKIP_SCAN	 0x00200000U /* skip lvmcache_label_scan in process_each_pv */
#define PROCESS_SKIP_ORPHAN_LOCK 0x00400000U /* skip lock_vol(VG_ORPHAN) in vg_read */

/* vg's "read_status" field */
#define FAILED_INCONSISTENT	0x00000001U
#define FAILED_LOCKING		0x00000002U
#define FAILED_NOTFOUND		0x00000004U
#define FAILED_READ_ONLY	0x00000008U
#define FAILED_EXPORTED		0x00000010U
#define FAILED_RESIZEABLE	0x00000020U
#define FAILED_CLUSTERED	0x00000040U
#define FAILED_ALLOCATION	0x00000080U
#define FAILED_EXIST		0x00000100U
#define FAILED_RECOVERY		0x00000200U
#define FAILED_SYSTEMID		0x00000400U
#define FAILED_LOCK_TYPE	0x00000800U
#define FAILED_LOCK_MODE	0x00001000U
#define SUCCESS			0x00000000U

#define VGMETADATACOPIES_ALL UINT32_MAX
#define VGMETADATACOPIES_UNMANAGED 0

#define vg_is_archived(vg)	(((vg)->status & ARCHIVED_VG) ? 1 : 0)

#define lv_is_locked(lv)	(((lv)->status & LOCKED) ? 1 : 0)
#define lv_is_partial(lv)	(((lv)->status & PARTIAL_LV) ? 1 : 0)
#define lv_is_virtual(lv)	(((lv)->status & VIRTUAL) ? 1 : 0)
#define lv_is_merging(lv)	(((lv)->status & MERGING) ? 1 : 0)
#define lv_is_merging_origin(lv) (lv_is_merging(lv) && (lv)->snapshot)
#define lv_is_snapshot(lv)	(((lv)->status & SNAPSHOT) ? 1 : 0)
#define lv_is_converting(lv)	(((lv)->status & CONVERTING) ? 1 : 0)
#define lv_is_external_origin(lv)	(((lv)->external_count > 0) ? 1 : 0)
#define lv_is_virtual_origin(lv) (((lv)->status & VIRTUAL_ORIGIN) ? 1 : 0)

#define lv_is_thin_volume(lv)	(((lv)->status & THIN_VOLUME) ? 1 : 0)
#define lv_is_thin_pool(lv)	(((lv)->status & THIN_POOL) ? 1 : 0)
#define lv_is_new_thin_pool(lv)	(lv_is_thin_pool(lv) && !first_seg(lv)->transaction_id)
#define lv_is_used_thin_pool(lv)	(lv_is_thin_pool(lv) && !dm_list_empty(&(lv)->segs_using_this_lv))
#define lv_is_thin_pool_data(lv)	(((lv)->status & THIN_POOL_DATA) ? 1 : 0)
#define lv_is_thin_pool_metadata(lv)	(((lv)->status & THIN_POOL_METADATA) ? 1 : 0)
#define lv_is_thin_type(lv)	(((lv)->status & (THIN_POOL | THIN_VOLUME | THIN_POOL_DATA | THIN_POOL_METADATA)) ? 1 : 0)

#define lv_is_mirrored(lv)	(((lv)->status & MIRRORED) ? 1 : 0)

#define lv_is_mirror_image(lv)	(((lv)->status & MIRROR_IMAGE) ? 1 : 0)
#define lv_is_mirror_log(lv)	(((lv)->status & MIRROR_LOG) ? 1 : 0)
#define lv_is_mirror(lv)	(((lv)->status & MIRROR) ? 1 : 0)
#define lv_is_mirror_type(lv)	(((lv)->status & (MIRROR | MIRROR_LOG | MIRROR_IMAGE)) ? 1 : 0)
#define lv_is_not_synced(lv)	(((lv)->status & LV_NOTSYNCED) ? 1 : 0)

#define lv_is_pending_delete(lv) (((lv)->status & LV_PENDING_DELETE) ? 1 : 0)
#define lv_is_error_when_full(lv) (((lv)->status & LV_ERROR_WHEN_FULL) ? 1 : 0)
#define lv_is_pvmove(lv)	(((lv)->status & PVMOVE) ? 1 : 0)

#define lv_is_raid(lv)		(((lv)->status & RAID) ? 1 : 0)
#define lv_is_raid_image(lv)	(((lv)->status & RAID_IMAGE) ? 1 : 0)
#define lv_is_raid_image_with_tracking(lv)	((lv_is_raid_image(lv) && !((lv)->status & LVM_WRITE)) ? 1 : 0)
#define lv_is_raid_metadata(lv)	(((lv)->status & RAID_META) ? 1 : 0)
#define lv_is_raid_type(lv)	(((lv)->status & (RAID | RAID_IMAGE | RAID_META)) ? 1 : 0)

#define lv_is_cache(lv)		(((lv)->status & CACHE) ? 1 : 0)
#define lv_is_cache_pool(lv)	(((lv)->status & CACHE_POOL) ? 1 : 0)
#define lv_is_used_cache_pool(lv)	(lv_is_cache_pool(lv) && !dm_list_empty(&(lv)->segs_using_this_lv))
#define lv_is_cache_pool_data(lv)	(((lv)->status & CACHE_POOL_DATA) ? 1 : 0)
#define lv_is_cache_pool_metadata(lv)	(((lv)->status & CACHE_POOL_METADATA) ? 1 : 0)
#define lv_is_cache_type(lv)	(((lv)->status & (CACHE | CACHE_POOL | CACHE_POOL_DATA | CACHE_POOL_METADATA)) ? 1 : 0)

#define lv_is_pool(lv)		(((lv)->status & (CACHE_POOL | THIN_POOL)) ? 1 : 0)
#define lv_is_pool_data(lv)		(((lv)->status & (CACHE_POOL_DATA | THIN_POOL_DATA)) ? 1 : 0)
#define lv_is_pool_metadata(lv)		(((lv)->status & (CACHE_POOL_METADATA | THIN_POOL_METADATA)) ? 1 : 0)
#define lv_is_pool_metadata_spare(lv)	(((lv)->status & POOL_METADATA_SPARE) ? 1 : 0)
#define lv_is_lockd_sanlock_lv(lv)	(((lv)->status & LOCKD_SANLOCK_LV) ? 1 : 0)

#define lv_is_vdo(lv)		(((lv)->status & LV_VDO) ? 1 : 0)
#define lv_is_vdo_pool(lv)	(((lv)->status & LV_VDO_POOL) ? 1 : 0)
#define lv_is_vdo_pool_data(lv)	(((lv)->status & LV_VDO_POOL_DATA) ? 1 : 0)
#define lv_is_vdo_type(lv)	(((lv)->status & (LV_VDO | LV_VDO_POOL | LV_VDO_POOL_DATA)) ? 1 : 0)

#define lv_is_removed(lv)	(((lv)->status & LV_REMOVED) ? 1 : 0)

/* Recognize component LV (matching lib/misc/lvm-string.c _lvname_has_reserved_component_string()) */
#define lv_is_component(lv) (lv_is_cache_origin(lv) || ((lv)->status & (\
	CACHE_POOL_DATA |\
	CACHE_POOL_METADATA |\
	LV_VDO_POOL_DATA |\
	MIRROR_IMAGE |\
	MIRROR_LOG |\
	RAID_IMAGE |\
	RAID_META |\
	THIN_POOL_DATA |\
	THIN_POOL_METADATA)) ? 1 : 0)

int lv_layout_and_role(struct dm_pool *mem, const struct logical_volume *lv,
		       struct dm_list **layout, struct dm_list **role);

/* Ordered list - see lv_manip.c */
typedef enum {
	AREA_UNASSIGNED,
	AREA_PV,
	AREA_LV
} area_type_t;

/* Whether or not to force an operation */
typedef enum {
	PROMPT = 0, /* Issue yes/no prompt to confirm operation */
	DONT_PROMPT = 1, /* Add more prompts */
	DONT_PROMPT_OVERRIDE = 2 /* Add even more dangerous prompts */
} force_t;

enum {
	MIRROR_LOG_CORE,
	MIRROR_LOG_DISK,
	MIRROR_LOG_MIRRORED,
};

typedef enum {
	THIN_ZERO_UNSELECTED = 0,
	THIN_ZERO_NO,
	THIN_ZERO_YES,
} thin_zero_t;

typedef enum {
	THIN_DISCARDS_UNSELECTED = 0,
	THIN_DISCARDS_IGNORE,
	THIN_DISCARDS_NO_PASSDOWN,
	THIN_DISCARDS_PASSDOWN,
} thin_discards_t;

typedef enum {
	CACHE_MODE_UNSELECTED = 0,
	CACHE_MODE_WRITETHROUGH,
	CACHE_MODE_WRITEBACK,
	CACHE_MODE_PASSTHROUGH,
} cache_mode_t;

/* ATM used for cache only */
typedef enum {
	CACHE_METADATA_FORMAT_UNSELECTED = 0,  /* On input means 'auto' */
	CACHE_METADATA_FORMAT_1,
	CACHE_METADATA_FORMAT_2,
} cache_metadata_format_t;

typedef enum {
	LOCK_TYPE_INVALID = -1,
	LOCK_TYPE_NONE = 0,
	LOCK_TYPE_CLVM = 1,
	LOCK_TYPE_DLM = 2,
	LOCK_TYPE_SANLOCK = 3,
} lock_type_t;

struct cmd_context;
struct format_handler;
struct labeller;

struct format_type {
	struct dm_list list;
	struct cmd_context *cmd;
	struct format_handler *ops;
	struct dm_list mda_ops; /* List of permissible mda ops. */
	struct labeller *labeller;
	const char *name;
	const char *alias;
	const char *orphan_vg_name;
	struct volume_group *orphan_vg; /* Only one ever exists. */
	uint32_t features;
	void *library;
	void *private;
};

struct pv_segment {
	struct dm_list list;	/* Member of pv->segments: ordered list
				 * covering entire data area on this PV */

	struct physical_volume *pv;
	uint32_t pe;
	uint32_t len;

	struct lv_segment *lvseg;	/* NULL if free space */
	uint32_t lv_area;	/* Index to area in LV segment */
};

#define pvseg_is_allocated(pvseg) ((pvseg)->lvseg ? 1 : 0)

/*
 * Properties of each format instance type.
 * The primary role of the format_instance is to temporarily store metadata
 * area information we are working with.
 */

/* Include any existing PV ("on-disk") mdas during format_instance initialisation. */
#define FMT_INSTANCE_MDAS		0x00000002U

/*
 * Include any auxiliary mdas during format_instance intialisation.
 * Currently, this includes metadata areas as defined by
 * metadata/dirs and metadata/raws setting.
 */
#define FMT_INSTANCE_AUX_MDAS		0x00000004U

/*
 * Include any other format-specific mdas during format_instance initialisation.
 * For example metadata areas used during backup/restore/archive handling.
 */
#define FMT_INSTANCE_PRIVATE_MDAS	0x00000008U

/*
 * Each VG has its own fid struct.  The fid for a VG describes where
 * the metadata for that VG can be found.  The lists hold mda locations.
 *
 * label scan finds the metadata locations (devs and offsets) for a VG,
 * and saves this info in lvmcache vginfo/info lists.
 *
 * vg_read() then creates an fid for a given VG, and the mda locations
 * from lvmcache are copied onto the fid lists.  Those mda locations
 * are read again by vg_read() to get VG metadata that is used to
 * create the 'vg' struct.
 */

struct format_instance {
	unsigned ref_count;	/* Refs to this fid from VG and PV structs */
	struct dm_pool *mem;

	uint32_t type;
	const struct format_type *fmt;

	/*
	 * Each mda in a vg is on exactly one of the below lists.
	 * MDAs on the 'in_use' list will be read from / written to
	 * disk, while MDAs on the 'ignored' list will not be read
	 * or written to.
	 */
	/* FIXME: Try to use the index only. Remove these lists. */
	struct dm_list metadata_areas_in_use;
	struct dm_list metadata_areas_ignored;
	struct dm_hash_table *metadata_areas_index;

	void *private;
};

/* There will be one area for each stripe */
struct lv_segment_area {
	area_type_t type;
	union {
		struct {
			struct pv_segment *pvseg;
		} pv;
		struct {
			struct logical_volume *lv;
			uint32_t le;
		} lv;
	} u;
};

struct lv_thin_message {
	struct dm_list list;		/* Chained list of messages */
	dm_thin_message_t type;		/* Use dm thin message datatype */
	union {
		struct logical_volume *lv; /* For: create_thin, create_snap, trim */
		uint32_t delete_id;	/* For delete, needs device_id */
	} u;
};

struct segment_type;

struct lv_segment {
	struct dm_list list;
	struct logical_volume *lv;

	const struct segment_type *segtype;
	uint32_t le;
	uint32_t len;
	uint32_t reshape_len;   /* For RAID: user hidden additional out of place reshaping length off area_len and len */

	uint64_t status;

	/* FIXME Fields depend on segment type */
	uint32_t stripe_size;	/* For stripe and RAID - in sectors */
	uint32_t writebehind;   /* For RAID (RAID1 only) */
	uint32_t min_recovery_rate; /* For RAID */
	uint32_t max_recovery_rate; /* For RAID */
	uint32_t data_offset;	/* For RAID: data offset in sectors on each data component image */
	uint32_t area_count;
	uint32_t area_len;
	uint32_t chunk_size;	/* For snapshots/thin_pool.  In sectors. */
				/* For thin_pool, 128..2097152. */
	struct logical_volume *origin;	/* snap and thin */
	struct generic_logical_volume *indirect_origin;
	struct logical_volume *merge_lv; /* thin, merge descendent lv into this ancestor */
	struct logical_volume *cow;
	struct dm_list origin_list;
	uint32_t region_size;	/* For raids/mirrors - in sectors */
	uint32_t data_copies;	/* For RAID: number of data copies (e.g. 3 for RAID 6 */
	uint32_t extents_copied;/* Number of extents synced for raids/mirrors */
	struct logical_volume *log_lv;
	struct lv_segment *pvmove_source_seg;
	void *segtype_private;

	struct dm_list tags;

	struct lv_segment_area *areas;
	struct lv_segment_area *meta_areas;	/* For RAID */
	struct logical_volume *metadata_lv;	/* For thin_pool */
	uint64_t transaction_id;		/* For thin_pool, thin */
	thin_zero_t zero_new_blocks;		/* For thin_pool */
	thin_discards_t discards;		/* For thin_pool */
	struct dm_list thin_messages;		/* For thin_pool */
	struct logical_volume *external_lv;	/* For thin */
	struct logical_volume *pool_lv;		/* For thin, cache */
	uint32_t device_id;			/* For thin, 24bit */

	cache_metadata_format_t cache_metadata_format;/* For cache_pool */
	cache_mode_t cache_mode;		/* For cache_pool */
	const char *policy_name;		/* For cache_pool */
	struct dm_config_node *policy_settings;	/* For cache_pool */
	unsigned cleaner_policy;		/* For cache */

	struct dm_vdo_target_params vdo_params;	/* For VDO-pool */
	uint32_t vdo_pool_header_size;		/* For VDO-pool */
	uint32_t vdo_pool_virtual_extents;	/* For VDO-pool */
};

#define seg_type(seg, s)	(seg)->areas[(s)].type
#define seg_pv(seg, s)		(seg)->areas[(s)].u.pv.pvseg->pv
#define seg_lv(seg, s)		(seg)->areas[(s)].u.lv.lv
#define seg_metalv(seg, s)	(seg)->meta_areas[(s)].u.lv.lv
#define seg_metatype(seg, s)	(seg)->meta_areas[(s)].type

struct pe_range {
	struct dm_list list;
	uint32_t start;		/* PEs */
	uint32_t count;		/* PEs */
};

struct pv_list {
	struct dm_list list;
	struct physical_volume *pv;
	struct dm_list *mdas;	/* Metadata areas */
	struct dm_list *pe_ranges;	/* Ranges of PEs e.g. for allocation */
};

struct lv_list {
	struct dm_list list;
	struct logical_volume *lv;
};

struct glv_list {
	struct dm_list list;
	struct generic_logical_volume *glv;
};

struct vg_list {
	struct dm_list list;
	struct volume_group *vg;
};

struct vgnameid_list {
	struct dm_list list;
	const char *vg_name;
	const char *vgid;
};

#define PV_PE_START_CALC ((uint64_t) -1) /* Calculate pe_start value */

/*
 * Values used by pv_create().
 */
struct pv_create_args {
	uint64_t size;
	uint64_t data_alignment;
	uint64_t data_alignment_offset;
	uint64_t label_sector;
	int pvmetadatacopies;
	uint64_t pvmetadatasize;
	unsigned metadataignore;

	/* used when restoring */
	struct id id;
	struct id *idp;
	uint64_t ba_start;
	uint64_t ba_size;
	uint64_t pe_start;
	uint32_t extent_count;
	uint32_t extent_size;
};

struct pvcreate_params {
	/*
	 * From argc and argv.
	 */
	char **pv_names;
	uint32_t pv_count;

	/*
	 * From command line args.
	 */
	int zero;
	force_t force;
	unsigned yes;

	/*
	 * From recovery-specific command line args.
	 */
	const char *restorefile; /* NULL if no --restorefile option */
	const char *uuid_str;    /* id in printable format, NULL if no id */

	/*
	 * Values used by pv_create().
	 */
	struct pv_create_args pva;

	/*
	 * Used for command processing.
	 */
	struct dm_list prompts;         /* pvcreate_prompt */
	struct dm_list arg_devices;     /* pvcreate_device, one for each pv_name */
	struct dm_list arg_process;     /* pvcreate_device, used for processing */
	struct dm_list arg_confirm;     /* pvcreate_device, used for processing */
	struct dm_list arg_create;      /* pvcreate_device, used for pvcreate */
	struct dm_list arg_remove;      /* pvcreate_device, used for pvremove */
	struct dm_list arg_fail;        /* pvcreate_device, failed to create */
	struct dm_list pvs;             /* pv_list, created and usable for vgcreate/vgextend */
	const char *orphan_vg_name;
	unsigned is_remove : 1;         /* is removing PVs, not creating */
	unsigned preserve_existing : 1;
	unsigned check_failed : 1;
};

struct lvresize_params {
	int argc;
	char **argv;

	const char *vg_name; /* only-used when VG is not yet opened (in /tools) */
	const char *lv_name;

	const struct segment_type *segtype;

	uint64_t poolmetadata_size;
	sign_t poolmetadata_sign;

	/* Per LV applied parameters */

	enum {
		LV_ANY = 0,
		LV_REDUCE = 1,
		LV_EXTEND = 2
	} resize;

	int use_policies;

	alloc_policy_t alloc;
	int yes;
	int force;
	int nosync;
	int nofsck;
	int resizefs;

	unsigned mirrors;
	uint32_t stripes;
	uint64_t stripe_size;

	uint32_t extents;
	uint64_t size;
	sign_t sign;
	percent_type_t percent;

	int approx_alloc;
	int extents_are_pes;	/* Is 'extents' counting PEs or LEs? */
	int size_changed;	/* Was there actually a size change */
};

void pvcreate_params_set_defaults(struct pvcreate_params *pp);

/*
 * Flags that indicate which warnings a library function should issue.
 */ 
#define WARN_PV_READ      0x00000001
#define WARN_INCONSISTENT 0x00000002
#define SKIP_RESCAN       0x00000004

/*
* Utility functions
*/
int vg_write(struct volume_group *vg);
int vg_commit(struct volume_group *vg);
void vg_revert(struct volume_group *vg);

/*
 * Add/remove LV to/from volume group
 */
int link_lv_to_vg(struct volume_group *vg, struct logical_volume *lv);
int unlink_lv_from_vg(struct logical_volume *lv);
void lv_set_visible(struct logical_volume *lv);
void lv_set_hidden(struct logical_volume *lv);

int get_vgnameids(struct cmd_context *cmd, struct dm_list *vgnameids,
		  const char *only_this_vgname, int include_internal);

int pv_write(struct cmd_context *cmd, struct physical_volume *pv, int allow_non_orphan);
int move_pv(struct volume_group *vg_from, struct volume_group *vg_to,
	    const char *pv_name);
int move_pvs_used_by_lv(struct volume_group *vg_from,
			struct volume_group *vg_to,
			const char *lv_name);
int is_global_vg(const char *vg_name);
int is_orphan_vg(const char *vg_name);
int is_real_vg(const char *vg_name);
int vg_missing_pv_count(const struct volume_group *vg);
int vgs_are_compatible(struct cmd_context *cmd,
		       struct volume_group *vg_from,
		       struct volume_group *vg_to);
uint32_t vg_lock_newname(struct cmd_context *cmd, const char *vgname);

int lv_resize(struct logical_volume *lv,
	      struct lvresize_params *lp,
	      struct dm_list *pvh);

/*
 * Return a handle to VG metadata.
 */
struct volume_group *vg_read_internal(struct cmd_context *cmd,
                                      const char *vgname, const char *vgid,
                                      uint32_t lockd_state, uint32_t warn_flags,
                                      int enable_repair,
                                      int *mdas_consistent);
struct volume_group *vg_read(struct cmd_context *cmd, const char *vg_name,
			     const char *vgid, uint32_t read_flags, uint32_t lockd_state);
struct volume_group *vg_read_for_update(struct cmd_context *cmd, const char *vg_name,
			 const char *vgid, uint32_t read_flags, uint32_t lockd_state);
struct volume_group *vg_read_orphans(struct cmd_context *cmd,
                                             uint32_t warn_flags,
                                             const char *orphan_vgname);
/* 
 * Test validity of a VG handle.
 */
uint32_t vg_read_error(struct volume_group *vg_handle);

/* pe_start and pe_end relate to any existing data so that new metadata
* areas can avoid overlap */
struct physical_volume *pv_create(const struct cmd_context *cmd,
				  struct device *dev, struct pv_create_args *pva);

int pv_resize_single(struct cmd_context *cmd,
			     struct volume_group *vg,
			     struct physical_volume *pv,
			     const uint64_t new_size,
			     int yes);

int pv_analyze(struct cmd_context *cmd, struct device *dev,
	       uint64_t label_sector);

/* FIXME: move internal to library */
uint32_t pv_list_extents_free(const struct dm_list *pvh);

int validate_new_vg_name(struct cmd_context *cmd, const char *vg_name);
int vg_validate(struct volume_group *vg);
struct volume_group *vg_create(struct cmd_context *cmd, const char *vg_name);
struct volume_group *vg_lock_and_create(struct cmd_context *cmd, const char *vg_name);
int vg_remove_mdas(struct volume_group *vg);
int vg_remove_check(struct volume_group *vg);
void vg_remove_pvs(struct volume_group *vg);
int vg_remove_direct(struct volume_group *vg);
int vg_remove(struct volume_group *vg);
int vg_rename(struct cmd_context *cmd, struct volume_group *vg,
	      const char *new_name);
int vg_extend_each_pv(struct volume_group *vg, struct pvcreate_params *pp);

int vgreduce_single(struct cmd_context *cmd, struct volume_group *vg,
			    struct physical_volume *pv, int commit);

int vg_change_tag(struct volume_group *vg, const char *tag, int add_tag);
int vg_split_mdas(struct cmd_context *cmd, struct volume_group *vg_from,
		  struct volume_group *vg_to);
/* FIXME: Investigate refactoring these functions to take a pv ISO pv_list */
void add_pvl_to_vgs(struct volume_group *vg, struct pv_list *pvl);
void del_pvl_from_vgs(struct volume_group *vg, struct pv_list *pvl);

/*
 * free_pv_fid() must be called on every struct physical_volume allocated
 * by pv_create, pv_read, find_pv_by_name or to free it when no longer required.
 */
void free_pv_fid(struct physical_volume *pv);

/* Manipulate LVs */
struct logical_volume *lv_create_empty(const char *name,
				       union lvid *lvid,
				       uint64_t status,
				       alloc_policy_t alloc,
				       struct volume_group *vg);

struct wipe_params {
	int do_zero;		/* should we do zeroing of LV start? */
	uint64_t zero_sectors;	/* sector count to zero */
	int zero_value;		/* zero-out with this value */
	int do_wipe_signatures;	/* should we wipe known signatures found on LV? */
	int yes;		/* answer yes automatically to all questions */
	force_t force;		/* force mode */
};

/* Zero out LV and/or wipe signatures */
int wipe_lv(struct logical_volume *lv, struct wipe_params params);

/* Wipe any signatures and zero first sector on @lv */
int activate_and_wipe_lv(struct logical_volume *lv, int commit);

/* Wipe any signatures and zero first sector of LVs listed on @lv_list */
int activate_and_wipe_lvlist(struct dm_list *lv_list, int commit);

int lv_change_tag(struct logical_volume *lv, const char *tag, int add_tag);

/* Reduce the size of an LV by extents */
int lv_reduce(struct logical_volume *lv, uint32_t extents);

/* Empty an LV prior to deleting it */
int lv_empty(struct logical_volume *lv);

/* Empty an LV and add error segment */
int replace_lv_with_error_segment(struct logical_volume *lv);

int lv_refresh_suspend_resume(const struct logical_volume *lv);

/* Entry point for all LV extent allocations */
int lv_extend(struct logical_volume *lv,
	      const struct segment_type *segtype,
	      uint32_t stripes, uint32_t stripe_size,
	      uint32_t mirrors, uint32_t region_size,
	      uint32_t extents,
	      struct dm_list *allocatable_pvs, alloc_policy_t alloc,
	      int approx_alloc);

/* lv must be part of lv->vg->lvs */
int lv_remove(struct logical_volume *lv);

/* historical_glv must be part of lv->vg->historical_lvs */
int historical_glv_remove(struct generic_logical_volume *historical_glv);

int lv_remove_single(struct cmd_context *cmd, struct logical_volume *lv,
		     force_t force, int suppress_remove_message);

int lv_remove_with_dependencies(struct cmd_context *cmd, struct logical_volume *lv,
				force_t force, unsigned level);

int lv_rename(struct cmd_context *cmd, struct logical_volume *lv,
	      const char *new_name);
int lv_rename_update(struct cmd_context *cmd, struct logical_volume *lv,
		     const char *new_name, int update_mda);

/* Updates and reloads metadata for given lv */
int lv_update_and_reload(struct logical_volume *lv);
int lv_update_and_reload_origin(struct logical_volume *lv);

uint32_t extents_from_size(struct cmd_context *cmd, uint64_t size,
			   uint32_t extent_size);
uint32_t extents_from_percent_size(struct volume_group *vg, const struct dm_list *pvh,
				   uint32_t extents, int roundup,
				   percent_type_t percent, uint64_t size);

struct logical_volume *find_pool_lv(const struct logical_volume *lv);
int pool_is_active(const struct logical_volume *lv);
int pool_supports_external_origin(const struct lv_segment *pool_seg, const struct logical_volume *external_lv);
int thin_pool_feature_supported(const struct logical_volume *lv, int feature);
int recalculate_pool_chunk_size_with_dev_hints(struct logical_volume *pool_lv,
					       int chunk_size_calc_policy);
int validate_cache_chunk_size(struct cmd_context *cmd, uint32_t chunk_size);
int validate_thin_pool_chunk_size(struct cmd_context *cmd, uint32_t chunk_size);
int validate_pool_chunk_size(struct cmd_context *cmd, const struct segment_type *segtype, uint32_t chunk_size);
int update_pool_lv(struct logical_volume *lv, int activate);
int get_default_allocation_thin_pool_chunk_size(struct cmd_context *cmd, struct profile *profile,
						uint32_t *chunk_size, int *chunk_size_calc_method);
int update_thin_pool_params(struct cmd_context *cmd,
			    struct profile *profile,
			    uint32_t extent_size,
			    const struct segment_type *segtype,
			    unsigned attr,
			    uint32_t pool_data_extents,
			    uint32_t *pool_metadata_extents,
			    int *chunk_size_calc_method, uint32_t *chunk_size,
			    thin_discards_t *discards, thin_zero_t *zero_new_blocks);
const char *get_pool_discards_name(thin_discards_t discards);
int set_pool_discards(thin_discards_t *discards, const char *str);
struct logical_volume *alloc_pool_metadata(struct logical_volume *pool_lv,
					   const char *name, uint32_t read_ahead,
					   uint32_t stripes, uint32_t stripe_size,
					   uint32_t extents, alloc_policy_t alloc,
					   struct dm_list *pvh);
int handle_pool_metadata_spare(struct volume_group *vg, uint32_t extents,
			       struct dm_list *pvh, int poolmetadataspare);
int vg_set_pool_metadata_spare(struct logical_volume *lv);
int vg_remove_pool_metadata_spare(struct volume_group *vg);

int attach_thin_external_origin(struct lv_segment *seg,
				struct logical_volume *external_lv);
int detach_thin_external_origin(struct lv_segment *seg);
int attach_pool_metadata_lv(struct lv_segment *pool_seg,
                            struct logical_volume *metadata_lv);
int detach_pool_metadata_lv(struct lv_segment *pool_seg,
                            struct logical_volume **metadata_lv);
int attach_pool_data_lv(struct lv_segment *pool_seg,
                        struct logical_volume *pool_data_lv);
int is_mirror_image_removable(struct logical_volume *mimage_lv, void *baton);

/*
 * Activation options
 */
typedef enum activation_change {
	CHANGE_AY = 0,  /* activate */
	CHANGE_AN = 1,  /* deactivate */
	CHANGE_AEY = 2, /* activate exclusively */
	CHANGE_ALY = 3, /* activate locally */
	CHANGE_ALN = 4, /* deactivate locally */
	CHANGE_AAY = 5, /* automatic activation */
	CHANGE_ASY = 6  /* activate shared */
} activation_change_t;

/* Returns true, when change activates device */
static inline int is_change_activating(activation_change_t change)
{
        return ((change != CHANGE_AN) && (change != CHANGE_ALN));
}

/* FIXME: refactor and reduce the size of this struct! */
struct lvcreate_params {
	/* flags */
	int snapshot; /* snap */
	int create_pool; /* pools */
	int zero; /* all */
	int wipe_signatures; /* all */
	int32_t major; /* all */
	int32_t minor; /* all */
	int log_count; /* mirror/RAID */
	int nosync; /* mirror/RAID */
	int pool_metadata_spare; /* pools */
	int type;   /* type arg is given */
	int temporary; /* temporary LV */
#define ACTIVATION_SKIP_SET		0x01 /* request to set LV activation skip flag state */
#define ACTIVATION_SKIP_SET_ENABLED	0x02 /* set the LV activation skip flag state to 'enabled' */
#define ACTIVATION_SKIP_IGNORE		0x04 /* request to ignore LV activation skip flag (if any) */
	int activation_skip; /* activation skip flags */
	activation_change_t activate; /* non-snapshot, non-mirror */
	thin_discards_t discards;     /* thin */
	thin_zero_t zero_new_blocks;
#define THIN_CHUNK_SIZE_CALC_METHOD_GENERIC 0x01
#define THIN_CHUNK_SIZE_CALC_METHOD_PERFORMANCE 0x02
	int thin_chunk_size_calc_policy;
	unsigned suppress_zero_warn : 1;
	unsigned needs_lockd_init : 1;

	const char *vg_name; /* only-used when VG is not yet opened (in /tools) */
	const char *lv_name; /* all */
	const char *origin_name; /* snap */
	const char *pool_name;   /* thin */

	const char *lock_args;

	uint32_t stripes; /* striped/RAID */
	uint32_t stripe_size; /* striped/RAID */
	uint32_t chunk_size; /* snapshot */
	uint32_t region_size; /* mirror/RAID */

	unsigned stripes_supplied; /* striped/RAID */
	unsigned stripe_size_supplied; /* striped/RAID */

	uint32_t mirrors; /* mirror/RAID */

	uint32_t min_recovery_rate; /* RAID */
	uint32_t max_recovery_rate; /* RAID */

	cache_metadata_format_t cache_metadata_format; /* cache */
	cache_mode_t cache_mode; /* cache */
	const char *policy_name; /* cache */
	struct dm_config_tree *policy_settings; /* cache */

	const struct segment_type *segtype; /* all */
	unsigned target_attr; /* all */

	/* size */
	uint32_t extents; /* all */
	uint32_t pool_metadata_extents; /* pools */
	uint64_t pool_metadata_size; /* pools */
	uint32_t pool_data_extents; /* pools */
	uint64_t pool_data_size; /* pools */
	uint32_t virtual_extents; /* snapshots, thins */
	struct dm_list *pvh; /* all */

	uint64_t permission; /* all */
	unsigned error_when_full; /* when segment supports it */
	uint32_t read_ahead; /* all */
	int approx_alloc;     /* all */
	alloc_policy_t alloc; /* all */
	struct dm_vdo_target_params vdo_params; /* vdo */

	struct dm_list tags;	/* all */

	int yes;
	force_t force;
};

struct logical_volume *lv_create_single(struct volume_group *vg,
					struct lvcreate_params *lp);

/*
 * The activation can be skipped for selected LVs. Some LVs are skipped
 * by default (e.g. thin snapshots), others can be skipped on demand by
 * overriding the default behaviour. The flag that causes the activation
 * skip on next activations is stored directly in metadata for each LV
 * as ACTIVATION_SKIP flag.
 */
void lv_set_activation_skip(struct logical_volume *lv, int override_default, int add_skip);
int lv_activation_skip(struct logical_volume *lv, activation_change_t activate,
		       int override_lv_skip_flag);

/*
 * Functions for layer manipulation
 */
int insert_layer_for_segments_on_pv(struct cmd_context *cmd,
				    struct logical_volume *lv_where,
				    struct logical_volume *layer_lv,
				    uint64_t status,
				    struct pv_list *pvl,
				    struct dm_list *lvs_changed);
int remove_layers_for_segments(struct cmd_context *cmd,
			       struct logical_volume *lv,
			       struct logical_volume *layer_lv,
			       uint64_t status_mask, struct dm_list *lvs_changed);
int remove_layers_for_segments_all(struct cmd_context *cmd,
				   struct logical_volume *layer_lv,
				   uint64_t status_mask,
				   struct dm_list *lvs_changed);
int split_parent_segments_for_layer(struct cmd_context *cmd,
				    struct logical_volume *layer_lv);
int remove_layer_from_lv(struct logical_volume *lv,
			 struct logical_volume *layer_lv);
struct logical_volume *insert_layer_for_lv(struct cmd_context *cmd,
					   struct logical_volume *lv_where,
					   uint64_t status,
					   const char *layer_suffix);

/* Find a PV within a given VG */
struct pv_list *find_pv_in_vg(const struct volume_group *vg,
			      const char *pv_name);
struct pv_list *find_pv_in_vg_by_uuid(const struct volume_group *vg,
				      const struct id *id);

/* Find an LV within a given VG */
struct lv_list *find_lv_in_vg(const struct volume_group *vg,
			      const char *lv_name);

/* FIXME Merge these functions with ones above */
struct logical_volume *find_lv(const struct volume_group *vg,
			       const char *lv_name);

struct generic_logical_volume *find_historical_glv(const struct volume_group *vg,
						    const char *historical_lv_name,
						    int check_removed_list,
						    struct glv_list **glvl_found);

int lv_name_is_used_in_vg(const struct volume_group *vg, const char *name, int *historical);

int lv_is_on_pv(struct logical_volume *lv, struct physical_volume *pv);
int lv_is_on_pvs(struct logical_volume *lv, struct dm_list *pvs);
int get_pv_list_for_lv(struct dm_pool *mem,
		       struct logical_volume *lv, struct dm_list *pvs);


/* Find LV segment containing given LE */
struct lv_segment *first_seg(const struct logical_volume *lv);
struct lv_segment *last_seg(const struct logical_volume *lv);
struct lv_segment *get_only_segment_using_this_lv(const struct logical_volume *lv);

/*
* Useful functions for managing snapshots.
*/
int lv_is_origin(const struct logical_volume *lv);
#define lv_is_thick_origin lv_is_origin

int lv_is_thin_origin(const struct logical_volume *lv, unsigned *snap_count);
int lv_is_thin_snapshot(const struct logical_volume *lv);

int lv_is_cow(const struct logical_volume *lv);
#define lv_is_thick_snapshot lv_is_cow

int lv_is_cache_origin(const struct logical_volume *lv);

int lv_is_merging_cow(const struct logical_volume *cow);
uint32_t cow_max_extents(const struct logical_volume *origin, uint32_t chunk_size);
int cow_has_min_chunks(const struct volume_group *vg, uint32_t cow_extents, uint32_t chunk_size);
int lv_is_cow_covering_origin(const struct logical_volume *lv);

/* Test if given LV is visible from user's perspective */
int lv_is_visible(const struct logical_volume *lv);

int lv_is_historical(const struct logical_volume *lv);

int pv_is_in_vg(struct volume_group *vg, struct physical_volume *pv);

/* Given a cow or thin LV, return the snapshot lv_segment that uses it */
struct lv_segment *find_snapshot(const struct logical_volume *lv);

/* Given a cow LV, return its origin */
struct logical_volume *origin_from_cow(const struct logical_volume *lv);

/* Given an internal snapshot LV, return its cow */
struct logical_volume *find_cow(const struct logical_volume *snap);

void init_snapshot_seg(struct lv_segment *seg, struct logical_volume *origin,
		       struct logical_volume *cow, uint32_t chunk_size, int merge);

void init_snapshot_merge(struct lv_segment *snap_seg, struct logical_volume *origin);

void clear_snapshot_merge(struct logical_volume *origin);

int vg_add_snapshot(struct logical_volume *origin, struct logical_volume *cow,
		    union lvid *lvid, uint32_t extent_count,
		    uint32_t chunk_size);

int vg_remove_snapshot(struct logical_volume *cow);

int validate_snapshot_origin(const struct logical_volume *origin_lv);


int vg_check_status(const struct volume_group *vg, uint64_t status);

int vg_check_pv_dev_block_sizes(const struct volume_group *vg);

/*
 * Check if the VG reached maximal LVs count (if set)
 */
int vg_max_lv_reached(struct volume_group *vg);

/*
* Mirroring functions
*/
int get_default_region_size(struct cmd_context *cmd);  /* in lv_manip.c */
struct lv_segment *find_mirror_seg(struct lv_segment *seg);
int lv_add_mirrors(struct cmd_context *cmd, struct logical_volume *lv,
		   uint32_t mirrors, uint32_t stripes, uint32_t stripe_size,
		   uint32_t region_size, uint32_t log_count,
		   struct dm_list *pvs, alloc_policy_t alloc, uint32_t flags);
int lv_split_mirror_images(struct logical_volume *lv, const char *split_lv_name,
			   uint32_t split_count, struct dm_list *removable_pvs);
int lv_remove_mirrors(struct cmd_context *cmd, struct logical_volume *lv,
		      uint32_t mirrors, uint32_t log_count,
		      int (*is_removable)(struct logical_volume *, void *),
		      void *removable_baton, uint64_t status_mask);
const char *get_mirror_log_name(int log_count);
int set_mirror_log_count(int *log_count, const char *mirrorlog);

int cluster_mirror_is_available(struct cmd_context *cmd);
int is_temporary_mirror_layer(const struct logical_volume *lv);
struct logical_volume * find_temporary_mirror(const struct logical_volume *lv);
uint32_t lv_mirror_count(const struct logical_volume *lv);

/* Remove CMIRROR_REGION_COUNT_LIMIT when http://bugzilla.redhat.com/682771 is fixed */
#define CMIRROR_REGION_COUNT_LIMIT (256*1024 * 8)
uint32_t adjusted_mirror_region_size(struct cmd_context *cmd,
				     uint32_t extent_size, uint32_t extents,
				     uint32_t region_size, int internal, int clustered);

int remove_mirrors_from_segments(struct logical_volume *lv,
				 uint32_t new_mirrors, uint64_t status_mask);
int add_mirrors_to_segments(struct cmd_context *cmd, struct logical_volume *lv,
			    uint32_t mirrors, uint32_t region_size,
			    struct dm_list *allocatable_pvs, alloc_policy_t alloc);

int remove_mirror_images(struct logical_volume *lv, uint32_t num_mirrors,
			 int (*is_removable)(struct logical_volume *, void *),
			 void *removable_baton, unsigned remove_log);
int add_mirror_images(struct cmd_context *cmd, struct logical_volume *lv,
		      uint32_t mirrors, uint32_t stripes, uint32_t stripe_size, uint32_t region_size,
		      struct dm_list *allocatable_pvs, alloc_policy_t alloc,
		      uint32_t log_count);
struct logical_volume *detach_mirror_log(struct lv_segment *mirrored_seg);
int attach_mirror_log(struct lv_segment *seg, struct logical_volume *log_lv);
int remove_mirror_log(struct cmd_context *cmd, struct logical_volume *lv,
		      struct dm_list *removable_pvs, int force);
struct logical_volume *prepare_mirror_log(struct logical_volume *lv,
					  int in_sync, uint32_t region_size,
					  struct dm_list *allocatable_pvs,
					  alloc_policy_t alloc);
int add_mirror_log(struct cmd_context *cmd, struct logical_volume *lv,
		   uint32_t log_count, uint32_t region_size,
		   struct dm_list *allocatable_pvs, alloc_policy_t alloc);

#if 0
/* FIXME: reconfigure_mirror_images: remove this code? */
int reconfigure_mirror_images(struct lv_segment *mirrored_seg, uint32_t num_mirrors,
			      struct dm_list *removable_pvs, unsigned remove_log);
#endif
int collapse_mirrored_lv(struct logical_volume *lv);
int shift_mirror_images(struct lv_segment *mirrored_seg, unsigned mimage);

/* ++  metadata/raid_manip.c */
int lv_is_raid_with_tracking(const struct logical_volume *lv);
uint32_t lv_raid_image_count(const struct logical_volume *lv);
int lv_raid_change_image_count(struct logical_volume *lv,
			       int yes,
			       uint32_t new_count,
			       uint32_t new_region_size,
			       struct dm_list *allocate_pvs);
int lv_raid_split(struct logical_volume *lv, int yes, const char *split_name,
		  uint32_t new_count, struct dm_list *splittable_pvs);
int lv_raid_split_and_track(struct logical_volume *lv,
			    int yes,
			    struct dm_list *splittable_pvs);
int lv_raid_merge(struct logical_volume *image_lv);
int lv_raid_convert(struct logical_volume *lv,
		    const struct segment_type *new_segtype,
		    int yes, int force,
		    const unsigned new_stripes,
		    const unsigned new_stripe_size_supplied,
		    const unsigned new_stripe_size,
		    const uint32_t new_region_size,
		    struct dm_list *allocate_pvs);
int lv_raid_rebuild(struct logical_volume *lv, struct dm_list *rebuild_pvs);
int lv_raid_replace(struct logical_volume *lv, int force,
		    struct dm_list *remove_pvs, struct dm_list *allocate_pvs);
int lv_raid_remove_missing(struct logical_volume *lv);
int partial_raid_lv_supports_degraded_activation(const struct logical_volume *clv);
uint32_t raid_rmeta_extents_delta(struct cmd_context *cmd,
				  uint32_t rimage_extents_cur, uint32_t rimage_extents_new,
				  uint32_t region_size, uint32_t extent_size);
uint32_t raid_rimage_extents(const struct segment_type *segtype,
			     uint32_t extents, uint32_t stripes, uint32_t data_copies);
uint32_t raid_ensure_min_region_size(const struct logical_volume *lv, uint64_t raid_size, uint32_t region_size);
int lv_raid_change_region_size(struct logical_volume *lv,
                               int yes, int force, uint32_t new_region_size);
int lv_raid_in_sync(const struct logical_volume *lv);
uint32_t lv_raid_data_copies(const struct segment_type *segtype, uint32_t area_count);
int lv_raid_free_reshape_space(const struct logical_volume *lv);
int lv_raid_clear_lv(struct logical_volume *lv, int commit);
int lv_raid_has_visible_sublvs(const struct logical_volume *lv);
/* --  metadata/raid_manip.c */

/* ++  metadata/cache_manip.c */
struct lv_status_cache {
	struct dm_pool *mem;
	struct dm_status_cache *cache;
	dm_percent_t data_usage;
	dm_percent_t metadata_usage;
	dm_percent_t dirty_usage;
};

const char *display_cache_mode(const struct lv_segment *seg);
const char *get_cache_mode_name(const struct lv_segment *pool_seg);
int set_cache_mode(cache_mode_t *mode, const char *cache_mode);
int cache_set_cache_mode(struct lv_segment *seg, cache_mode_t mode);
int cache_set_metadata_format(struct lv_segment *seg, cache_metadata_format_t format);
int cache_set_policy(struct lv_segment *seg, const char *name,
		     const struct dm_config_tree *settings);
int cache_set_params(struct lv_segment *seg,
		     uint32_t chunk_size,
		     cache_metadata_format_t format,
		     cache_mode_t mode,
		     const char *policy_name,
		     const struct dm_config_tree *policy_settings);
void cache_check_for_warns(const struct lv_segment *seg);
int update_cache_pool_params(struct cmd_context *cmd,
			     struct profile *profile,
			     uint32_t extent_size,
			     const struct segment_type *segtype,
			     unsigned attr,
			     uint32_t pool_data_extents,
			     uint32_t *pool_metadata_extents,
			     int *chunk_size_calc_method, uint32_t *chunk_size);
int validate_lv_cache_chunk_size(struct logical_volume *pool_lv, uint32_t chunk_size);
int validate_lv_cache_create_pool(const struct logical_volume *pool_lv);
int validate_lv_cache_create_origin(const struct logical_volume *origin_lv);
struct logical_volume *lv_cache_create(struct logical_volume *pool_lv,
				       struct logical_volume *origin_lv);
int lv_cache_wait_for_clean(struct logical_volume *cache_lv, int *is_clean);
int lv_cache_remove(struct logical_volume *cache_lv);
int wipe_cache_pool(struct logical_volume *cache_pool_lv);
/* --  metadata/cache_manip.c */


/* ++  metadata/vdo_manip.c */
struct lv_status_vdo {
	struct dm_pool *mem;
	struct dm_vdo_status *vdo;
	uint64_t data_blocks_used;	/* grabbed from /sys/kvdo */
	uint64_t logical_blocks_used;	/* grabbed from /sys/kvdo */
	dm_percent_t usage;
	dm_percent_t saving;
	dm_percent_t data_usage;
};

const char *get_vdo_compression_state_name(enum dm_vdo_compression_state state);
const char *get_vdo_index_state_name(enum dm_vdo_index_state state);
const char *get_vdo_operating_mode_name(enum dm_vdo_operating_mode mode);
uint64_t get_vdo_pool_virtual_size(const struct lv_segment *vdo_pool_seg);
int parse_vdo_pool_status(struct dm_pool *mem, const struct logical_volume *vdo_pool_lv,
			  const char *params, struct lv_status_vdo *status);
struct logical_volume *convert_vdo_pool_lv(struct logical_volume *data_lv,
					   const struct dm_vdo_target_params *vtp,
					   uint32_t *virtual_extents);
int get_vdo_write_policy(enum dm_vdo_write_policy *vwp, const char *policy);
int fill_vdo_target_params(struct cmd_context *cmd,
			   struct dm_vdo_target_params *vtp,
			   struct profile *profile);
/* --  metadata/vdo_manip.c */

struct logical_volume *find_pvmove_lv(struct volume_group *vg,
				      struct device *dev, uint64_t lv_type);
const struct logical_volume *find_pvmove_lv_in_lv(const struct logical_volume *lv);
const char *get_pvmove_pvname_from_lv(const struct logical_volume *lv);
const char *get_pvmove_pvname_from_lv_mirr(const struct logical_volume *lv_mirr);
struct dm_list *lvs_using_lv(struct cmd_context *cmd, struct volume_group *vg,
			  struct logical_volume *lv);

uint32_t find_free_lvnum(struct logical_volume *lv);
dm_percent_t copy_percent(const struct logical_volume *lv);
char *generate_lv_name(struct volume_group *vg, const char *format,
		       char *buffer, size_t len);
char *top_level_lv_name(struct volume_group *vg, const char *lv_name);

struct generic_logical_volume *get_or_create_glv(struct dm_pool *mem, struct logical_volume *lv, int *glv_created);
struct glv_list *get_or_create_glvl(struct dm_pool *mem, struct logical_volume *lv, int *glv_created);

/*
* Begin skeleton for external LVM library
*/
int pv_change_metadataignore(struct physical_volume *pv, uint32_t mda_ignored);


int vg_flag_write_locked(struct volume_group *vg);
int vg_check_write_mode(struct volume_group *vg);
#define vg_is_clustered(vg) ((vg_status((vg)) & CLUSTERED) ? 1 : 0)
#define vg_is_exported(vg) ((vg_status((vg)) & EXPORTED_VG) ? 1 : 0)
#define vg_is_resizeable(vg) ((vg_status((vg)) & RESIZEABLE_VG) ? 1 : 0)

int lv_has_unknown_segments(const struct logical_volume *lv);
int vg_has_unknown_segments(const struct volume_group *vg);

int vg_mark_partial_lvs(struct volume_group *vg, int clear);

struct vgcreate_params {
	const char *vg_name;
	uint32_t extent_size;
	size_t max_pv;
	size_t max_lv;
	alloc_policy_t alloc;
	int clustered; /* FIXME: put this into a 'status' variable instead? */
	uint32_t vgmetadatacopies;
	const char *system_id;
	const char *lock_type;
	const char *lock_args;
};

int validate_major_minor(const struct cmd_context *cmd,
			 const struct format_type *fmt,
			 int32_t major, int32_t minor);
int vgcreate_params_validate(struct cmd_context *cmd,
			     struct vgcreate_params *vp);

int validate_vg_rename_params(struct cmd_context *cmd,
			      const char *vg_name_old,
			      const char *vg_name_new);

int is_lockd_type(const char *lock_type);
int vg_is_shared(const struct volume_group *vg);

int is_system_id_allowed(struct cmd_context *cmd, const char *system_id);

int vg_strip_outdated_historical_lvs(struct volume_group *vg);

#endif
