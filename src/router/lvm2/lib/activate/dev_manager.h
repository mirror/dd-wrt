/*
 * Copyright (C) 2002-2004 Sistina Software, Inc. All rights reserved.  
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

#ifndef _LVM_DEV_MANAGER_H
#define _LVM_DEV_MANAGER_H

#include "lib/metadata/metadata-exported.h"

struct logical_volume;
struct lv_activate_opts;
struct volume_group;
struct cmd_context;
struct dev_manager;
struct dm_info;
struct device;
struct lv_seg_status;

int read_only_lv(const struct logical_volume *lv, const struct lv_activate_opts *laopts, const char *layer);

/*
 * Constructor and destructor.
 */
struct dev_manager *dev_manager_create(struct cmd_context *cmd,
				       const char *vg_name,
				       unsigned track_pvmove_deps);
void dev_manager_destroy(struct dev_manager *dm);
void dev_manager_release(void);
void dev_manager_exit(void);

/*
 * The device handler is responsible for creating all the layered
 * dm devices, and ensuring that all constraints are maintained
 * (eg, an origin is created before its snapshot, but is not
 * unsuspended until the snapshot is also created.)
 */
int dev_manager_info(struct cmd_context *cmd, const struct logical_volume *lv,
		     const char *layer,
		     int with_open_count, int with_read_ahead,
		     struct dm_info *dminfo, uint32_t *read_ahead,
		     struct lv_seg_status *seg_status);

int dev_manager_snapshot_percent(struct dev_manager *dm,
				 const struct logical_volume *lv,
				 dm_percent_t *percent);
int dev_manager_mirror_percent(struct dev_manager *dm,
			       const struct logical_volume *lv, int wait,
			       dm_percent_t *percent, uint32_t *event_nr);
int dev_manager_raid_status(struct dev_manager *dm,
			    const struct logical_volume *lv,
			    struct dm_status_raid **status);
int dev_manager_raid_message(struct dev_manager *dm,
			     const struct logical_volume *lv,
			     const char *msg);
int dev_manager_cache_status(struct dev_manager *dm,
			     const struct logical_volume *lv,
			     struct lv_status_cache **status);
int dev_manager_thin_pool_status(struct dev_manager *dm,
				 const struct logical_volume *lv,
				 struct dm_status_thin_pool **status,
				 int flush);
int dev_manager_thin_pool_percent(struct dev_manager *dm,
				  const struct logical_volume *lv,
				  int metadata, dm_percent_t *percent);
int dev_manager_thin_percent(struct dev_manager *dm,
			     const struct logical_volume *lv,
			     int mapped, dm_percent_t *percent);
int dev_manager_thin_device_id(struct dev_manager *dm,
			       const struct logical_volume *lv,
			       uint32_t *device_id);
int dev_manager_vdo_pool_status(struct dev_manager *dm,
				const struct logical_volume *lv,
				int flush,
				char **vdo_params,
				struct lv_status_vdo **vdo_status);
int dev_manager_suspend(struct dev_manager *dm, const struct logical_volume *lv,
			struct lv_activate_opts *laopts, int lockfs, int flush_required);
int dev_manager_activate(struct dev_manager *dm, const struct logical_volume *lv,
			 struct lv_activate_opts *laopts);
int dev_manager_preload(struct dev_manager *dm, const struct logical_volume *lv,
			struct lv_activate_opts *laopts, int *flush_required);
int dev_manager_deactivate(struct dev_manager *dm, const struct logical_volume *lv);
int dev_manager_transient(struct dev_manager *dm, const struct logical_volume *lv) __attribute__((nonnull(1, 2)));

int dev_manager_mknodes(const struct logical_volume *lv);

/*
 * Put the desired changes into effect.
 */
int dev_manager_execute(struct dev_manager *dm);

int dev_manager_device_uses_vg(struct device *dev,
			       struct volume_group *vg);

#endif
