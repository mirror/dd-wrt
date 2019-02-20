/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2012 Red Hat, Inc. All rights reserved.
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
#ifndef _LVM_LV_H
#define _LVM_LV_H

#include "lib/metadata/vg.h"

union lvid;
struct lv_segment;
enum activation_change;

struct logical_volume {
	union lvid lvid;
	const char *name;

	struct volume_group *vg;

	uint64_t status;
	alloc_policy_t alloc;
	struct profile *profile;
	uint32_t read_ahead;
	int32_t major;
	int32_t minor;

	uint64_t size;		/* Sectors visible */
	uint32_t le_count;	/* Logical extents visible */

	uint32_t origin_count;
	uint32_t external_count;
	struct dm_list snapshot_segs;
	struct lv_segment *snapshot;

	struct dm_list segments;
	struct dm_list tags;
	struct dm_list segs_using_this_lv;
	struct dm_list indirect_glvs; /* For keeping track of historical LVs in ancestry chain */

	/*
	 * this_glv variable is used as a helper for handling historical LVs.
	 * If this LVs has no role at all in keeping track of historical LVs,
	 * the this_glv variable is NULL. See also comments for struct
	 * generic_logical_volume and struct historical_logical_volume below.
	 */
	struct generic_logical_volume *this_glv;

	uint64_t timestamp;
	unsigned new_lock_args:1;
	const char *hostname;
	const char *lock_args;
};

/*
 * With the introduction of tracking historical LVs, we need to make
 * a difference between live LV (struct logical_volume) and historical LV
 * (struct historical_logical_volume). To minimize the impact of this change
 * and to minimize the changes needed in the existing  code, we use a
 * little trick here - when processing LVs (e.g. while reporting LV
 * properties), each historical LV is represented as dummy LV which is
 * an instance of struct logical_volume with all its properties set to
 * blank (hence "dummy LV") and with this_glv pointing to the struct
 * historical_logical_volume. This way all the existing code working with
 * struct logical_volume will see this historical LV as dummy live LV while
 * the code that needs to recognize between live and historical LV will
 * check this_glv first and then it will work either with the live
 * or historical LV properties appropriately.
 */
struct generic_logical_volume;

/*
 * historical logical volume is an LV that has been removed already.
 * This is used to keep track of LV history.
 */
struct historical_logical_volume {
	union lvid lvid;
	const char *name;
	struct volume_group *vg;
	uint64_t timestamp;
	uint64_t timestamp_removed;
	struct generic_logical_volume *indirect_origin;
	struct dm_list indirect_glvs; /* list of struct generic_logical_volume */
	unsigned checked:1; /* set if this historical LV has been checked for validity */
	unsigned valid:1;   /* historical LV is valid if there's at least one live LV among ancestors */
};

struct generic_logical_volume {
	int is_historical;
	union {
		struct logical_volume *live;			/* is_historical=0 */
		struct historical_logical_volume *historical;	/* is_historical=1 */
	};
};

struct lv_with_info_and_seg_status;

/* LV dependencies */
struct logical_volume *lv_parent(const struct logical_volume *lv);
struct logical_volume *lv_convert_lv(const struct logical_volume *lv);
struct logical_volume *lv_origin_lv(const struct logical_volume *lv);
struct logical_volume *lv_mirror_log_lv(const struct logical_volume *lv);
struct logical_volume *lv_data_lv(const struct logical_volume *lv);
struct logical_volume *lv_convert(const struct logical_volume *lv);
struct logical_volume *lv_origin(const struct logical_volume *lv);
struct logical_volume *lv_mirror_log(const struct logical_volume *lv);
struct logical_volume *lv_data(const struct logical_volume *lv);
struct logical_volume *lv_metadata_lv(const struct logical_volume *lv);
struct logical_volume *lv_pool_lv(const struct logical_volume *lv);

/* LV properties */
uint64_t lv_size(const struct logical_volume *lv);
uint64_t lvseg_size(const struct lv_segment *seg);
uint64_t lvseg_chunksize(const struct lv_segment *seg);
uint64_t lv_origin_size(const struct logical_volume *lv);
uint64_t lv_metadata_size(const struct logical_volume *lv);
struct profile *lv_config_profile(const struct logical_volume *lv);
const char *lv_layer(const struct logical_volume *lv);
const struct logical_volume *lv_lock_holder(const struct logical_volume *lv);
const struct logical_volume *lv_committed(const struct logical_volume *lv);
int lv_mirror_image_in_sync(const struct logical_volume *lv);
int lv_raid_image_in_sync(const struct logical_volume *lv);
int lv_raid_healthy(const struct logical_volume *lv);
const char *lvseg_name(const struct lv_segment *seg);
uint64_t lvseg_start(const struct lv_segment *seg);
struct dm_list *lvseg_devices(struct dm_pool *mem, const struct lv_segment *seg);
char *lvseg_devices_str(struct dm_pool *mem, const struct lv_segment *seg);
struct dm_list *lvseg_metadata_devices(struct dm_pool *mem, const struct lv_segment *seg);
char *lvseg_metadata_devices_str(struct dm_pool *mem, const struct lv_segment *seg);
struct dm_list *lvseg_seg_pe_ranges(struct dm_pool *mem, const struct lv_segment *seg);
char *lvseg_seg_pe_ranges_str(struct dm_pool *mem, const struct lv_segment *seg);
struct dm_list *lvseg_seg_le_ranges(struct dm_pool *mem, const struct lv_segment *seg);
char *lvseg_seg_le_ranges_str(struct dm_pool *mem, const struct lv_segment *seg);
struct dm_list *lvseg_seg_metadata_le_ranges(struct dm_pool *mem, const struct lv_segment *seg);
char *lvseg_seg_metadata_le_ranges_str(struct dm_pool *mem, const struct lv_segment *seg);

/* LV kernel properties */
int lv_kernel_major(const struct logical_volume *lv);
int lv_kernel_minor(const struct logical_volume *lv);
uint32_t lv_kernel_read_ahead(const struct logical_volume *lv);
char *lvseg_kernel_discards_dup(struct dm_pool *mem, const struct lv_segment *seg);

/* LV modification functions */
int lv_set_creation(struct logical_volume *lv,
		    const char *hostname, uint64_t timestamp);
int lv_active_change(struct cmd_context *cmd, struct logical_volume *lv,
		     enum activation_change activate);

/* LV dup functions */
char *lv_attr_dup_with_info_and_seg_status(struct dm_pool *mem, const struct lv_with_info_and_seg_status *lvdm);
char *lv_attr_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_uuid_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_tags_dup(const struct logical_volume *lv);
char *lv_path_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_dmpath_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_move_pv_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_move_pv_uuid_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_convert_lv_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_convert_lv_uuid_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_mirror_log_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_mirror_log_uuid_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_data_lv_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_data_lv_uuid_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_metadata_lv_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_metadata_lv_uuid_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_pool_lv_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_pool_lv_uuid_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_modules_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_name_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_fullname_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_parent_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_origin_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_origin_uuid_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lvseg_segtype_dup(struct dm_pool *mem, const struct lv_segment *seg);
char *lvseg_discards_dup(struct dm_pool *mem, const struct lv_segment *seg);
char *lvseg_cachemode_dup(struct dm_pool *mem, const struct lv_segment *seg);
char *lvseg_monitor_dup(struct dm_pool *mem, const struct lv_segment *seg);
char *lvseg_tags_dup(const struct lv_segment *seg);
char *lv_creation_time_dup(struct dm_pool *mem, const struct logical_volume *lv, int iso_mode);
char *lv_removal_time_dup(struct dm_pool *mem, const struct logical_volume *lv, int iso_mode);
char *lv_host_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_active_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_profile_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_lock_args_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lvseg_kernel_discards_dup_with_info_and_seg_status(struct dm_pool *mem, const struct lv_with_info_and_seg_status *lvdm);
char *lv_time_dup(struct dm_pool *mem, const struct logical_volume *lv, int iso_mode);

typedef enum {
	PERCENT_GET_DATA = 0,
	PERCENT_GET_METADATA,
	PERCENT_GET_DIRTY
} percent_get_t;
dm_percent_t lvseg_percent_with_info_and_seg_status(const struct lv_with_info_and_seg_status *lvdm,
						    percent_get_t type);

#endif /* _LVM_LV_H */
