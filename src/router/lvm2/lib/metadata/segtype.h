/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2017 Red Hat, Inc. All rights reserved.
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

#ifndef _SEGTYPES_H
#define _SEGTYPES_H

#include "lib/metadata/metadata-exported.h"

struct segtype_handler;
struct cmd_context;
struct dm_config_tree;
struct lv_segment;
struct lv_activate_opts;
struct formatter;
struct dm_config_node;
struct dev_manager;

/* Feature flags */
#define SEG_CAN_SPLIT		(1ULL <<  0)
#define SEG_AREAS_STRIPED	(1ULL <<  1)
#define SEG_AREAS_MIRRORED	(1ULL <<  2)
#define SEG_SNAPSHOT		(1ULL <<  3)
/* #define SEG_FORMAT1_SUPPORT	(1ULL <<  4) */
#define SEG_VIRTUAL		(1ULL <<  5)
#define SEG_CANNOT_BE_ZEROED	(1ULL <<  6)
#define SEG_MONITORED		(1ULL <<  7)
#define SEG_RAID		(1ULL << 10)
#define SEG_THIN_POOL		(1ULL << 11)
#define SEG_THIN_VOLUME		(1ULL << 12)
#define SEG_CACHE		(1ULL << 13)
#define SEG_CACHE_POOL		(1ULL << 14)
#define SEG_MIRROR		(1ULL << 15)
#define SEG_ONLY_EXCLUSIVE	(1ULL << 16) /* In cluster only exlusive activation */
#define SEG_CAN_ERROR_WHEN_FULL	(1ULL << 17)

#define SEG_RAID0		(1ULL << 18)
#define SEG_RAID0_META		(1ULL << 19)
#define SEG_RAID1		(1ULL << 20)
#define SEG_RAID10_NEAR		(1ULL << 21)
#define SEG_RAID10		SEG_RAID10_NEAR
#define SEG_RAID4		(1ULL << 22)
#define SEG_RAID5_N		(1ULL << 23)
#define SEG_RAID5_LA		(1ULL << 24)
#define SEG_RAID5_LS		(1ULL << 25)
#define SEG_RAID5_RA		(1ULL << 26)
#define SEG_RAID5_RS		(1ULL << 27)
#define SEG_RAID5		SEG_RAID5_LS
#define SEG_RAID6_NC		(1ULL << 28)
#define SEG_RAID6_NR		(1ULL << 29)
#define SEG_RAID6_ZR		(1ULL << 30)
#define SEG_RAID6_LA_6		(1ULL << 31)
#define SEG_RAID6_LS_6		(1ULL << 32)
#define SEG_RAID6_RA_6		(1ULL << 33)
#define SEG_RAID6_RS_6		(1ULL << 34)
#define SEG_RAID6_N_6		(1ULL << 35)
#define SEG_RAID6		SEG_RAID6_ZR

#define SEG_STRIPED_TARGET	(1ULL << 39)
#define SEG_LINEAR_TARGET	(1ULL << 40)
#define SEG_VDO			(1ULL << 41)
#define SEG_VDO_POOL		(1ULL << 42)

#define SEG_UNKNOWN		(1ULL << 63)

#define SEG_TYPE_NAME_LINEAR		"linear"
#define SEG_TYPE_NAME_STRIPED		"striped"
#define SEG_TYPE_NAME_MIRROR		"mirror"
#define SEG_TYPE_NAME_SNAPSHOT		"snapshot"
#define SEG_TYPE_NAME_THIN		"thin"
#define SEG_TYPE_NAME_THIN_POOL		"thin-pool"
#define SEG_TYPE_NAME_CACHE		"cache"
#define SEG_TYPE_NAME_CACHE_POOL	"cache-pool"
#define SEG_TYPE_NAME_ERROR		"error"
#define SEG_TYPE_NAME_FREE		"free"
#define SEG_TYPE_NAME_ZERO		"zero"
#define SEG_TYPE_NAME_VDO		"vdo"
#define SEG_TYPE_NAME_VDO_POOL		"vdo-pool"
#define SEG_TYPE_NAME_RAID		"raid"
#define SEG_TYPE_NAME_RAID0		"raid0"
#define SEG_TYPE_NAME_RAID0_META	"raid0_meta"
#define SEG_TYPE_NAME_RAID1		"raid1"
#define SEG_TYPE_NAME_RAID10		"raid10"
#define SEG_TYPE_NAME_RAID10_NEAR	"raid10_near"
#define SEG_TYPE_NAME_RAID4		"raid4"
#define SEG_TYPE_NAME_RAID5		"raid5"
#define SEG_TYPE_NAME_RAID5_N		"raid5_n"
#define SEG_TYPE_NAME_RAID5_LA		"raid5_la"
#define SEG_TYPE_NAME_RAID5_LS		"raid5_ls"
#define SEG_TYPE_NAME_RAID5_RA		"raid5_ra"
#define SEG_TYPE_NAME_RAID5_RS		"raid5_rs"
#define SEG_TYPE_NAME_RAID6		"raid6"
#define SEG_TYPE_NAME_RAID6_NC		"raid6_nc"
#define SEG_TYPE_NAME_RAID6_NR		"raid6_nr"
#define SEG_TYPE_NAME_RAID6_ZR		"raid6_zr"
#define SEG_TYPE_NAME_RAID6_LA_6	"raid6_la_6"
#define SEG_TYPE_NAME_RAID6_LS_6	"raid6_ls_6"
#define SEG_TYPE_NAME_RAID6_RA_6	"raid6_ra_6"
#define SEG_TYPE_NAME_RAID6_RS_6	"raid6_rs_6"
#define SEG_TYPE_NAME_RAID6_N_6		"raid6_n_6"

#define segtype_is_linear(segtype)	(!strcmp((segtype)->name, SEG_TYPE_NAME_LINEAR))
#define segtype_is_striped_target(segtype)	((segtype)->flags & SEG_STRIPED_TARGET ? 1 : 0)
#define segtype_is_cache(segtype)	((segtype)->flags & SEG_CACHE ? 1 : 0)
#define segtype_is_cache_pool(segtype)	((segtype)->flags & SEG_CACHE_POOL ? 1 : 0)
#define segtype_is_mirrored(segtype)	((segtype)->flags & SEG_AREAS_MIRRORED ? 1 : 0)
#define segtype_is_mirror(segtype)	((segtype)->flags & SEG_MIRROR ? 1 : 0)
#define segtype_is_pool(segtype)	((segtype)->flags & (SEG_CACHE_POOL | SEG_THIN_POOL)  ? 1 : 0)
#define segtype_is_raid0(segtype)	((segtype)->flags & SEG_RAID0 ? 1 : 0)
#define segtype_is_raid0_meta(segtype)	((segtype)->flags & SEG_RAID0_META ? 1 : 0)
#define segtype_is_any_raid0(segtype)	((segtype)->flags & (SEG_RAID0 | SEG_RAID0_META) ? 1 : 0)
#define segtype_is_raid(segtype)	((segtype)->flags & SEG_RAID ? 1 : 0)
#define segtype_is_raid1(segtype)	((segtype)->flags & SEG_RAID1 ? 1 : 0)
#define segtype_is_raid4(segtype)	((segtype)->flags & SEG_RAID4 ? 1 : 0)
#define segtype_is_any_raid5(segtype)	((segtype)->flags & \
					 (SEG_RAID5_LS|SEG_RAID5_LA|SEG_RAID5_RS|SEG_RAID5_RA|SEG_RAID5_N) ? 1 : 0)
#define segtype_is_raid5_n(segtype)	((segtype)->flags & SEG_RAID5_N ? 1 : 0)
#define segtype_is_raid5_la(segtype)	((segtype)->flags & SEG_RAID5_LA ? 1 : 0)
#define segtype_is_raid5_ra(segtype)	((segtype)->flags & SEG_RAID5_RA ? 1 : 0)
#define segtype_is_raid5_ls(segtype)	((segtype)->flags & SEG_RAID5_LS ? 1 : 0)
#define segtype_is_raid5_rs(segtype)	((segtype)->flags & SEG_RAID5_RS ? 1 : 0)
#define segtype_is_any_raid6(segtype)	((segtype)->flags & \
					 (SEG_RAID6_ZR|SEG_RAID6_NC|SEG_RAID6_NR| \
					  SEG_RAID6_LA_6|SEG_RAID6_LS_6|SEG_RAID6_RA_6|SEG_RAID6_RS_6|SEG_RAID6_N_6) ? 1 : 0)
#define segtype_is_raid6_nc(segtype)	((segtype)->flags & SEG_RAID6_NC ? 1 : 0)
#define segtype_is_raid6_nr(segtype)	((segtype)->flags & SEG_RAID6_NR ? 1 : 0)
#define segtype_is_raid6_n_6(segtype)	((segtype)->flags & SEG_RAID6_N_6 ? 1 : 0)
#define segtype_is_raid6_zr(segtype)	((segtype)->flags & SEG_RAID6_ZR ? 1 : 0)
#define segtype_is_raid6_ls_6(segtype)	((segtype)->flags & SEG_RAID6_LS_6 ? 1 : 0)
#define segtype_is_raid6_rs_6(segtype)	((segtype)->flags & SEG_RAID6_RS_6 ? 1 : 0)
#define segtype_is_raid6_la_6(segtype)	((segtype)->flags & SEG_RAID6_LA_6 ? 1 : 0)
#define segtype_is_raid6_ra_6(segtype)	((segtype)->flags & SEG_RAID6_RA_6 ? 1 : 0)
#define segtype_is_raid10(segtype)	((segtype)->flags & SEG_RAID10 ? 1 : 0)
#define segtype_is_raid10_near(segtype)	((segtype)->flags & SEG_RAID10_NEAR ? 1 : 0)
/* FIXME: once raid10_{far,offset} supported */
#define segtype_is_raid10_far(segtype)		0 /* FIXME ((segtype)->flags & SEG_RAID10_FAR ? 1 : 0 */
#define segtype_is_raid10_offset(segtype)	0 /* FIXME ((segtype)->flags & SEG_RAID10_OFFSET ? 1 : 0 */
#define segtype_is_any_raid10(segtype)	(segtype_is_raid10(segtype) || segtype_is_raid10_near(segtype) || segtype_is_raid10_far(segtype) || segtype_is_raid10_offset(segtype))
#define segtype_is_raid_with_meta(segtype)	(segtype_is_raid(segtype) && !segtype_is_raid0(segtype))
#define segtype_is_striped_raid(segtype)        (segtype_is_raid(segtype) && !segtype_is_raid1(segtype))
#define segtype_is_reshapable_raid(segtype)     ((segtype_is_striped_raid(segtype) && !segtype_is_any_raid0(segtype)) || segtype_is_raid10_near(segtype) || segtype_is_raid10_offset(segtype))
#define segtype_is_snapshot(segtype)	((segtype)->flags & SEG_SNAPSHOT ? 1 : 0)
#define segtype_is_striped(segtype)	((segtype)->flags & SEG_AREAS_STRIPED ? 1 : 0)
#define segtype_is_thin(segtype)	((segtype)->flags & (SEG_THIN_POOL|SEG_THIN_VOLUME) ? 1 : 0)
#define segtype_is_thin_pool(segtype)	((segtype)->flags & SEG_THIN_POOL ? 1 : 0)
#define segtype_is_thin_volume(segtype)	((segtype)->flags & SEG_THIN_VOLUME ? 1 : 0)
#define segtype_is_vdo(segtype)		((segtype)->flags & SEG_VDO ? 1 : 0)
#define segtype_is_vdo_pool(segtype)	((segtype)->flags & SEG_VDO_POOL ? 1 : 0)
#define segtype_is_virtual(segtype)	((segtype)->flags & SEG_VIRTUAL ? 1 : 0)
#define segtype_is_unknown(segtype)	((segtype)->flags & SEG_UNKNOWN ? 1 : 0)

#define segtype_can_split(segtype)	((segtype)->flags & SEG_CAN_SPLIT ? 1 : 0)
#define segtype_cannot_be_zeroed(segtype) ((segtype)->flags & SEG_CANNOT_BE_ZEROED ? 1 : 0)
#define segtype_monitored(segtype)	((segtype)->flags & SEG_MONITORED ? 1 : 0)
#define segtype_only_exclusive(segtype)	((segtype)->flags & SEG_ONLY_EXCLUSIVE ? 1 : 0)
#define segtype_can_error_when_full(segtype)	((segtype)->flags & SEG_CAN_ERROR_WHEN_FULL ? 1 : 0)

#define segtype_supports_stripe_size(segtype)	\
	((segtype_is_striped(segtype) || segtype_is_mirror(segtype) || \
	  segtype_is_cache(segtype) || segtype_is_cache_pool(segtype) || \
	  segtype_is_thin(segtype) || segtype_is_snapshot(segtype) || \
	  (segtype_is_striped_raid(segtype))) ? 1 : 0)

#define seg_is_striped_target(seg)	segtype_is_striped_target((seg)->segtype)
#define seg_is_cache(seg)	segtype_is_cache((seg)->segtype)
#define seg_is_cache_pool(seg)	segtype_is_cache_pool((seg)->segtype)
#define seg_is_used_cache_pool(seg) (seg_is_cache_pool(seg) && (!dm_list_empty(&(seg->lv)->segs_using_this_lv)))
#define seg_is_linear(seg)	(seg_is_striped(seg) && ((seg)->area_count == 1))
#define seg_is_mirror(seg)	segtype_is_mirror((seg)->segtype)
#define seg_is_mirrored(seg)	segtype_is_mirrored((seg)->segtype)
#define seg_is_pool(seg)	segtype_is_pool((seg)->segtype)
#define seg_is_raid0(seg)	segtype_is_raid0((seg)->segtype)
#define seg_is_raid0_meta(seg)	segtype_is_raid0_meta((seg)->segtype)
#define seg_is_any_raid0(seg)	segtype_is_any_raid0((seg)->segtype)
#define seg_is_raid(seg)	segtype_is_raid((seg)->segtype)
#define seg_is_raid1(seg)	segtype_is_raid1((seg)->segtype)
#define seg_is_raid4(seg)	segtype_is_raid4((seg)->segtype)
#define seg_is_any_raid5(seg)	segtype_is_any_raid5((seg)->segtype)
#define seg_is_raid5_n(seg)	segtype_is_raid5_n((seg)->segtype)
#define seg_is_raid5_la(seg)	segtype_is_raid5_la((seg)->segtype)
#define seg_is_raid5_ra(seg)	segtype_is_raid5_ra((seg)->segtype)
#define seg_is_raid5_ls(seg)	segtype_is_raid5_ls((seg)->segtype)
#define seg_is_raid5_rs(seg)	segtype_is_raid5_rs((seg)->segtype)
#define seg_is_any_raid6(seg)	segtype_is_any_raid6((seg)->segtype)
#define seg_is_raid6_zr(seg)	segtype_is_raid6_zr((seg)->segtype)
#define seg_is_raid6_nr(seg)	segtype_is_raid6_nr((seg)->segtype)
#define seg_is_raid6_nc(seg)	segtype_is_raid6_nc((seg)->segtype)
#define seg_is_raid6_n_6(seg)	segtype_is_raid6_n_6((seg)->segtype)
#define seg_is_any_raid10(seg)	segtype_is_any_raid10((seg)->segtype)
#define seg_is_raid10(seg)	segtype_is_raid10((seg)->segtype)
#define seg_is_raid10_near(seg)	segtype_is_raid10_near((seg)->segtype)
#define seg_is_raid_with_meta(seg)	segtype_is_raid_with_meta((seg)->segtype)
#define seg_is_striped_raid(seg)	segtype_is_striped_raid((seg)->segtype)
#define	seg_is_reshapable_raid(seg)	segtype_is_reshapable_raid((seg)->segtype)
#define seg_is_snapshot(seg)	segtype_is_snapshot((seg)->segtype)
#define seg_is_striped(seg)	segtype_is_striped((seg)->segtype)
#define seg_is_thin(seg)	segtype_is_thin((seg)->segtype)
#define seg_is_thin_pool(seg)	segtype_is_thin_pool((seg)->segtype)
#define seg_is_thin_volume(seg)	segtype_is_thin_volume((seg)->segtype)
#define seg_is_vdo(seg)		segtype_is_vdo((seg)->segtype)
#define seg_is_vdo_pool(seg)	segtype_is_vdo_pool((seg)->segtype)
#define seg_is_virtual(seg)	segtype_is_virtual((seg)->segtype)
#define seg_unknown(seg)	segtype_is_unknown((seg)->segtype)
#define seg_can_split(seg)	segtype_can_split((seg)->segtype)
#define seg_cannot_be_zeroed(seg)	segtype_cannot_be_zeroed((seg)->segtype)
#define seg_monitored(seg)	segtype_monitored((seg)->segtype)
#define seg_only_exclusive(seg)	segtype_only_exclusive((seg)->segtype)
#define seg_can_error_when_full(seg)	segtype_can_error_when_full((seg)->segtype)

struct segment_type {
	struct dm_list list;		/* Internal */

	uint64_t flags;
	uint32_t parity_devs;		/* Parity drives required by segtype */

	struct segtype_handler *ops;
	const char *name;
	const char *dso;

	void *library;			/* lvm_register_segtype() sets this. */
	void *private;			/* For the segtype handler to use. */
};

struct segtype_handler {
	const char *(*name) (const struct lv_segment * seg);
	const char *(*target_name) (const struct lv_segment *seg,
				    const struct lv_activate_opts *laopts);
	void (*display) (const struct lv_segment * seg);
	int (*text_export) (const struct lv_segment * seg,
			    struct formatter * f);
	int (*text_import_area_count) (const struct dm_config_node * sn,
				       uint32_t *area_count);
	int (*text_import) (struct lv_segment * seg,
			    const struct dm_config_node * sn,
			    struct dm_hash_table * pv_hash);
	int (*merge_segments) (struct lv_segment * seg1,
			       struct lv_segment * seg2);
	int (*add_target_line) (struct dev_manager *dm, struct dm_pool *mem,
				struct cmd_context *cmd, void **target_state,
				struct lv_segment *seg,
				const struct lv_activate_opts *laopts,
				struct dm_tree_node *node, uint64_t len,
				uint32_t *pvmove_mirror_count);
	int (*target_status_compatible) (const char *type);
	int (*check_transient_status) (struct dm_pool *mem,
				       struct lv_segment *seg, char *params);
	int (*target_percent) (void **target_state,
			       dm_percent_t *percent,
			       struct dm_pool * mem,
			       struct cmd_context *cmd,
			       struct lv_segment *seg, char *params,
			       uint64_t *total_numerator,
			       uint64_t *total_denominator);
	int (*target_present) (struct cmd_context *cmd,
			       const struct lv_segment *seg,
			       unsigned *attributes);
	int (*modules_needed) (struct dm_pool *mem,
			       const struct lv_segment *seg,
			       struct dm_list *modules);
	void (*destroy) (struct segment_type * segtype);
	int (*target_monitored) (struct lv_segment *seg, int *pending, int *monitored);
	int (*target_monitor_events) (struct lv_segment *seg, int events);
	int (*target_unmonitor_events) (struct lv_segment *seg, int events);
};

struct segment_type *get_segtype_from_string(struct cmd_context *cmd,
					     const char *str);
struct segment_type *get_segtype_from_flag(struct cmd_context *cmd,
					   uint64_t flag);

struct segtype_library;
int lvm_register_segtype(struct segtype_library *seglib,
			 struct segment_type *segtype);

struct segment_type *init_linear_segtype(struct cmd_context *cmd);
struct segment_type *init_striped_segtype(struct cmd_context *cmd);
struct segment_type *init_zero_segtype(struct cmd_context *cmd);
struct segment_type *init_error_segtype(struct cmd_context *cmd);
struct segment_type *init_free_segtype(struct cmd_context *cmd);
struct segment_type *init_unknown_segtype(struct cmd_context *cmd,
					  const char *name);

#define RAID_FEATURE_RAID10			(1U << 0) /* version 1.3 */
#define RAID_FEATURE_RAID0			(1U << 1) /* version 1.7 */
#define RAID_FEATURE_RESHAPING			(1U << 2) /* version 1.8 */
#define RAID_FEATURE_RAID4			(1U << 3) /* ! version 1.8 or 1.9.0 */
#define RAID_FEATURE_SHRINK			(1U << 4) /* version 1.9.0 */
#define RAID_FEATURE_RESHAPE			(1U << 5) /* version 1.10.1 */
/*
 * RAID_FEATURE_NEW_DEVICES_ACCEPT_REBUILD
 * This signifies a behavioral change in dm-raid.  Prior to upstream kernel
 * commit 33e53f068, the kernel would refuse to allow 'rebuild' CTR args to
 * be submitted when other devices in the array had uninitialized superblocks.
 * After the commit, these parameters were allowed.
 *
 * The most obvious useful case of this new behavior is up-converting a
 * linear device to RAID1.  A new superblock is allocated for the linear dev
 * and it will be uninitialized, while all the new images are specified for
 * 'rebuild'.  This valid scenario would not have been allowed prior to
 * commit 33e53f068.
 *
 * Commit 33e53f068 did not bump the dm-raid version number.  So it exists
 * in some, but not all 1.8.1 versions of dm-raid.  The only way to be
 * certain the new behavior exists is to check for version 1.9.0.
 */
#define RAID_FEATURE_NEW_DEVICES_ACCEPT_REBUILD	(1U << 6) /* version 1.9.0 */

#ifdef RAID_INTERNAL
int init_raid_segtypes(struct cmd_context *cmd, struct segtype_library *seglib);
#endif

#define THIN_FEATURE_DISCARDS			(1U << 0)
#define THIN_FEATURE_EXTERNAL_ORIGIN		(1U << 1)
#define THIN_FEATURE_HELD_ROOT			(1U << 2)
#define THIN_FEATURE_BLOCK_SIZE			(1U << 3)
#define THIN_FEATURE_DISCARDS_NON_POWER_2	(1U << 4)
#define THIN_FEATURE_METADATA_RESIZE		(1U << 5)
#define THIN_FEATURE_EXTERNAL_ORIGIN_EXTEND	(1U << 6)
#define THIN_FEATURE_ERROR_IF_NO_SPACE		(1U << 7)

#ifdef THIN_INTERNAL
int init_thin_segtypes(struct cmd_context *cmd, struct segtype_library *seglib);
#endif

#ifdef CACHE_INTERNAL
int init_cache_segtypes(struct cmd_context *cmd, struct segtype_library *seglib);
#endif

#ifdef VDO_INTERNAL
int init_vdo_segtypes(struct cmd_context *cmd, struct segtype_library *seglib);
#endif

#define CACHE_FEATURE_POLICY_MQ			(1U << 0)
#define CACHE_FEATURE_POLICY_SMQ		(1U << 1)
#define CACHE_FEATURE_METADATA2			(1U << 2)

#define SNAPSHOT_FEATURE_FIXED_LEAK		(1U << 0) /* version 1.12 */

#ifdef SNAPSHOT_INTERNAL
struct segment_type *init_snapshot_segtype(struct cmd_context *cmd);
#endif

#define MIRROR_LOG_CLUSTERED			(1U << 0)

#ifdef MIRRORED_INTERNAL
struct segment_type *init_mirrored_segtype(struct cmd_context *cmd);
#endif

#ifdef CRYPT_INTERNAL
struct segment_type *init_crypt_segtype(struct cmd_context *cmd);
#endif

#endif
