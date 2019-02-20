/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2013 Red Hat, Inc. All rights reserved.
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
#ifndef _LVM_VG_H
#define _LVM_VG_H

#include "lib/uuid/uuid.h"
#include "device_mapper/all.h"

struct cmd_context;
struct format_instance;
struct logical_volume;

typedef enum {
	ALLOC_INVALID,
	ALLOC_CONTIGUOUS,
	ALLOC_CLING,
	ALLOC_CLING_BY_TAGS,	/* Internal - never written or displayed. */
	ALLOC_NORMAL,
	ALLOC_ANYWHERE,
	ALLOC_INHERIT
} alloc_policy_t;

#define MAX_EXTENT_COUNT  (UINT32_MAX)

struct volume_group {
	struct cmd_context *cmd;
	struct dm_pool *vgmem;
	struct format_instance *fid;
	const struct format_type *original_fmt;	/* Set when processing backup files */
	struct lvmcache_vginfo *vginfo;
	uint32_t seqno;		/* Metadata sequence number */
	unsigned skip_validate_lock_args : 1;

	/*
	 * The parsed committed (on-disk) copy of this VG; is NULL if this VG is committed
	 * version (i.e. vg_committed == NULL *implies* this is the committed copy,
	 * there is no guarantee that if this VG is the same as the committed one
	 * this will be NULL). The pointer is maintained by calls to
	 * _vg_update_vg_committed.
	 */
	struct volume_group *vg_committed;
	struct volume_group *vg_precommitted;

	alloc_policy_t alloc;
	struct profile *profile;
	uint64_t status;

	struct id id;
	const char *name;
	const char *old_name;		/* Set during vgrename and vgcfgrestore */
	const char *system_id;
	const char *lock_type;
	const char *lock_args;

	uint32_t extent_size;
	uint32_t extent_count;
	uint32_t free_count;

	uint32_t max_lv;
	uint32_t max_pv;

	/* physical volumes */
	uint32_t pv_count;
	struct dm_list pvs;

	/*
	 * List of physical volumes that were used in vgextend but do not carry
	 * a PV label yet. They need to be pvcreate'd at vg_write time.
	 */

	struct dm_list pv_write_list; /* struct pv_list */

	/*
	 * logical volumes
	 * The following relationship should always hold:
	 * dm_list_size(lvs) = user visible lv_count + snapshot_count + other invisible LVs
	 *
	 * Snapshots consist of 2 instances of "struct logical_volume":
	 * - cow (lv_name is visible to the user)
	 * - snapshot (lv_name is 'snapshotN')
	 *
	 * Mirrors consist of multiple instances of "struct logical_volume":
	 * - one for the mirror log
	 * - one for each mirror leg
	 * - one for the user-visible mirror LV
	 */
	struct dm_list lvs;
	struct dm_list historical_lvs;

	struct dm_list tags;

	/*
	 * FIXME: Move the next fields into a different struct?
	 */

	/*
	 * List of removed logical volumes by _lv_reduce.
	 */
	struct dm_list removed_lvs;

	/*
	 * List of removed historical logical volumes by historical_glv_remove.
	 */
	struct dm_list removed_historical_lvs;

	/*
	 * List of removed physical volumes by pvreduce.
	 * They have to get cleared on vg_commit.
	 */
	struct dm_list removed_pvs;
	uint32_t open_mode; /* FIXME: read or write - check lock type? */

	/*
	 * Store result of the last vg_read().
	 * 0 for success else appropriate FAILURE_* bits set.
	 */
	uint32_t read_status;
	uint32_t mda_copies; /* target number of mdas for this VG */

	struct dm_hash_table *hostnames; /* map of creation hostnames */
	struct logical_volume *pool_metadata_spare_lv; /* one per VG */
	struct logical_volume *sanlock_lv; /* one per VG */
};

struct volume_group *alloc_vg(const char *pool_name, struct cmd_context *cmd,
			      const char *vg_name);

/*
 * release_vg() must be called on every struct volume_group allocated
 * by vg_create() or vg_read_internal() to free it when no longer required.
 */
void release_vg(struct volume_group *vg);
void free_orphan_vg(struct volume_group *vg);

char *vg_fmt_dup(const struct volume_group *vg);
char *vg_name_dup(const struct volume_group *vg);
char *vg_system_id_dup(const struct volume_group *vg);
char *vg_lock_type_dup(const struct volume_group *vg);
char *vg_lock_args_dup(const struct volume_group *vg);
uint32_t vg_seqno(const struct volume_group *vg);
uint64_t vg_status(const struct volume_group *vg);
int vg_set_alloc_policy(struct volume_group *vg, alloc_policy_t alloc);
int vg_set_system_id(struct volume_group *vg, const char *system_id);
int vg_set_lock_type(struct volume_group *vg, const char *lock_type);
uint64_t vg_size(const struct volume_group *vg);
uint64_t vg_free(const struct volume_group *vg);
uint64_t vg_extent_size(const struct volume_group *vg);
int vg_check_new_extent_size(const struct format_type *fmt, uint32_t new_extent_size);
int vg_set_extent_size(struct volume_group *vg, uint32_t new_extent_size);
uint64_t vg_extent_count(const struct volume_group *vg);
uint64_t vg_free_count(const struct volume_group *vg);
uint64_t vg_pv_count(const struct volume_group *vg);
uint64_t vg_max_pv(const struct volume_group *vg);
int vg_set_max_pv(struct volume_group *vg, uint32_t max_pv);
uint64_t vg_max_lv(const struct volume_group *vg);
int vg_set_max_lv(struct volume_group *vg, uint32_t max_lv);
uint32_t vg_mda_count(const struct volume_group *vg);
uint32_t vg_mda_used_count(const struct volume_group *vg);
uint32_t vg_mda_copies(const struct volume_group *vg);
int vg_set_mda_copies(struct volume_group *vg, uint32_t mda_copies);
char *vg_profile_dup(const struct volume_group *vg);

/*
 * Returns visible LV count - number of LVs from user perspective
 */
unsigned vg_visible_lvs(const struct volume_group *vg);

/*
 * Count snapshot LVs.
 */
unsigned snapshot_count(const struct volume_group *vg);

uint64_t vg_mda_size(const struct volume_group *vg);
uint64_t vg_mda_free(const struct volume_group *vg);
char *vg_attr_dup(struct dm_pool *mem, const struct volume_group *vg);
char *vg_uuid_dup(const struct volume_group *vg);
char *vg_tags_dup(const struct volume_group *vg);

#endif /* _LVM_VG_H */
