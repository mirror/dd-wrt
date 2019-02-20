/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2008 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_CACHE_H
#define _LVM_CACHE_H

#include "lib/device/dev-cache.h"
#include "lib/device/dev-type.h"
#include "lib/uuid/uuid.h"
#include "lib/label/label.h"
#include "lib/locking/locking.h"

#define ORPHAN_PREFIX VG_ORPHANS
#define ORPHAN_VG_NAME(fmt) ORPHAN_PREFIX "_" fmt

/* LVM specific per-volume info */
/* Eventual replacement for struct physical_volume perhaps? */

struct cmd_context;
struct format_type;
struct volume_group;
struct physical_volume;
struct dm_config_tree;
struct format_instance;
struct metadata_area;
struct disk_locn;

struct lvmcache_vginfo;

/*
 * vgsummary represents a summary of the VG that is read
 * without a lock.  The info does not come through vg_read(),
 * but through reading mdas.  It provides information about
 * the VG that is needed to lock the VG and then read it fully
 * with vg_read(), after which the VG summary should be checked
 * against the full VG metadata to verify it was correct (since
 * it was read without a lock.)
 *
 * Once read, vgsummary information is saved in lvmcache_vginfo.
 */
struct lvmcache_vgsummary {
	const char *vgname;
	struct id vgid;
	uint64_t vgstatus;
	char *creation_host;
	const char *system_id;
	const char *lock_type;
	uint32_t mda_checksum;
	size_t mda_size;
	int zero_offset;
	int seqno;
};

int lvmcache_init(struct cmd_context *cmd);

void lvmcache_destroy(struct cmd_context *cmd, int retain_orphans, int reset);

int lvmcache_label_scan(struct cmd_context *cmd);
int lvmcache_label_rescan_vg(struct cmd_context *cmd, const char *vgname, const char *vgid);

/* Add/delete a device */
struct lvmcache_info *lvmcache_add(struct labeller *labeller, const char *pvid,
				   struct device *dev,
				   const char *vgname, const char *vgid,
				   uint32_t vgstatus);
int lvmcache_add_orphan_vginfo(const char *vgname, struct format_type *fmt);
void lvmcache_del(struct lvmcache_info *info);
void lvmcache_del_dev(struct device *dev);

/* Update things */
int lvmcache_update_vgname_and_id(struct lvmcache_info *info,
				  struct lvmcache_vgsummary *vgsummary);
int lvmcache_update_vg(struct volume_group *vg, unsigned precommitted);

void lvmcache_lock_vgname(const char *vgname, int read_only);
void lvmcache_unlock_vgname(const char *vgname);

/* Queries */
const struct format_type *lvmcache_fmt_from_vgname(struct cmd_context *cmd, const char *vgname, const char *vgid, unsigned revalidate_labels);
int lvmcache_lookup_mda(struct lvmcache_vgsummary *vgsummary);

/* Decrement and test if there are still vg holders in vginfo. */
int lvmcache_vginfo_holders_dec_and_test_for_zero(struct lvmcache_vginfo *vginfo);

struct lvmcache_vginfo *lvmcache_vginfo_from_vgname(const char *vgname,
					   const char *vgid);
struct lvmcache_vginfo *lvmcache_vginfo_from_vgid(const char *vgid);
struct lvmcache_info *lvmcache_info_from_pvid(const char *pvid, struct device *dev, int valid_only);
const char *lvmcache_vgname_from_vgid(struct dm_pool *mem, const char *vgid);
const char *lvmcache_vgid_from_vgname(struct cmd_context *cmd, const char *vgname);
struct device *lvmcache_device_from_pvid(struct cmd_context *cmd, const struct id *pvid, uint64_t *label_sector);
const char *lvmcache_vgname_from_info(struct lvmcache_info *info);
const struct format_type *lvmcache_fmt_from_info(struct lvmcache_info *info);
int lvmcache_vgs_locked(void);

int lvmcache_get_vgnameids(struct cmd_context *cmd, int include_internal,
                          struct dm_list *vgnameids);

/* Returns list of struct dm_str_list containing pool-allocated copy of pvids */
struct dm_list *lvmcache_get_pvids(struct cmd_context *cmd, const char *vgname,
				const char *vgid);

void lvmcache_drop_metadata(const char *vgname, int drop_precommitted);
void lvmcache_commit_metadata(const char *vgname);

int lvmcache_fid_add_mdas(struct lvmcache_info *info, struct format_instance *fid,
			  const char *id, int id_len);
int lvmcache_fid_add_mdas_pv(struct lvmcache_info *info, struct format_instance *fid);
int lvmcache_fid_add_mdas_vg(struct lvmcache_vginfo *vginfo, struct format_instance *fid);
int lvmcache_populate_pv_fields(struct lvmcache_info *info,
				struct volume_group *vg,
				struct physical_volume *pv);
int lvmcache_check_format(struct lvmcache_info *info, const struct format_type *fmt);
void lvmcache_del_mdas(struct lvmcache_info *info);
void lvmcache_del_das(struct lvmcache_info *info);
void lvmcache_del_bas(struct lvmcache_info *info);
int lvmcache_add_mda(struct lvmcache_info *info, struct device *dev,
		     uint64_t start, uint64_t size, unsigned ignored);
int lvmcache_add_da(struct lvmcache_info *info, uint64_t start, uint64_t size);
int lvmcache_add_ba(struct lvmcache_info *info, uint64_t start, uint64_t size);

void lvmcache_set_ext_version(struct lvmcache_info *info, uint32_t version);
uint32_t lvmcache_ext_version(struct lvmcache_info *info);
void lvmcache_set_ext_flags(struct lvmcache_info *info, uint32_t flags);
uint32_t lvmcache_ext_flags(struct lvmcache_info *info);

const struct format_type *lvmcache_fmt(struct lvmcache_info *info);
struct label *lvmcache_get_label(struct lvmcache_info *info);
struct label *lvmcache_get_dev_label(struct device *dev);
int lvmcache_has_dev_info(struct device *dev);

void lvmcache_update_pv(struct lvmcache_info *info, struct physical_volume *pv,
			const struct format_type *fmt);
int lvmcache_update_das(struct lvmcache_info *info, struct physical_volume *pv);
int lvmcache_update_bas(struct lvmcache_info *info, struct physical_volume *pv);
int lvmcache_foreach_mda(struct lvmcache_info *info,
			 int (*fun)(struct metadata_area *, void *),
			 void *baton);

int lvmcache_foreach_da(struct lvmcache_info *info,
			int (*fun)(struct disk_locn *, void *),
			void *baton);

int lvmcache_foreach_ba(struct lvmcache_info *info,
			int (*fun)(struct disk_locn *, void *),
			void *baton);

int lvmcache_foreach_pv(struct lvmcache_vginfo *vginfo,
			int (*fun)(struct lvmcache_info *, void *), void * baton);

uint64_t lvmcache_device_size(struct lvmcache_info *info);
void lvmcache_set_device_size(struct lvmcache_info *info, uint64_t size);
struct device *lvmcache_device(struct lvmcache_info *info);
int lvmcache_is_orphan(struct lvmcache_info *info);
unsigned lvmcache_mda_count(struct lvmcache_info *info);
int lvmcache_vgid_is_cached(const char *vgid);
uint64_t lvmcache_smallest_mda_size(struct lvmcache_info *info);

int lvmcache_found_duplicate_pvs(void);

void lvmcache_pvscan_duplicate_check(struct cmd_context *cmd);

int lvmcache_get_unused_duplicate_devs(struct cmd_context *cmd, struct dm_list *head);

int vg_has_duplicate_pvs(struct volume_group *vg);

int lvmcache_contains_lock_type_sanlock(struct cmd_context *cmd);

void lvmcache_get_max_name_lengths(struct cmd_context *cmd,
			unsigned *pv_max_name_len, unsigned *vg_max_name_len);

int lvmcache_vg_is_foreign(struct cmd_context *cmd, const char *vgname, const char *vgid);

void lvmcache_lock_ordering(int enable);

int lvmcache_dev_is_unchosen_duplicate(struct device *dev);

void lvmcache_remove_unchosen_duplicate(struct device *dev);

int lvmcache_pvid_in_unchosen_duplicates(const char *pvid);

int lvmcache_get_vg_devs(struct cmd_context *cmd,
			 struct lvmcache_vginfo *vginfo,
			 struct dm_list *devs);
void lvmcache_set_independent_location(const char *vgname);

int lvmcache_scan_mismatch(struct cmd_context *cmd, const char *vgname, const char *vgid);

/*
 * These are clvmd-specific functions and are not related to lvmcache.
 * FIXME: rename these with a clvm_ prefix in place of lvmcache_
 */
void lvmcache_save_vg(struct volume_group *vg, int precommitted);
struct volume_group *lvmcache_get_saved_vg(const char *vgid, int precommitted);
struct volume_group *lvmcache_get_saved_vg_latest(const char *vgid);
void lvmcache_drop_saved_vgid(const char *vgid);

int dev_in_device_list(struct device *dev, struct dm_list *head);

#endif
