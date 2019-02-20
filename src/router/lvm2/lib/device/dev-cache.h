/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_DEV_CACHE_H
#define _LVM_DEV_CACHE_H

#include "lib/device/device.h"
#include "lib/misc/lvm-wrappers.h"

struct cmd_context;

/*
 * predicate for devices.
 */
struct dev_filter {
	int (*passes_filter) (struct cmd_context *cmd, struct dev_filter *f, struct device *dev);
	void (*destroy) (struct dev_filter *f);
	void (*wipe) (struct dev_filter *f);
	void *private;
	unsigned use_count;
};

int dev_cache_index_devs(void);
struct dm_list *dev_cache_get_dev_list_for_vgid(const char *vgid);
struct dm_list *dev_cache_get_dev_list_for_lvid(const char *lvid);

/*
 * The global device cache.
 */
int dev_cache_init(struct cmd_context *cmd);
int dev_cache_exit(void);

/*
 * Returns number of open devices.
 */
int dev_cache_check_for_open_devices(void);

void dev_cache_scan(void);
int dev_cache_has_scanned(void);

int dev_cache_add_dir(const char *path);
struct device *dev_cache_get(struct cmd_context *cmd, const char *name, struct dev_filter *f);
const char *dev_cache_filtered_reason(const char *name);

// TODO
struct device *dev_cache_get_by_devt(struct cmd_context *cmd, dev_t device, struct dev_filter *f);

void dev_set_preferred_name(struct dm_str_list *sl, struct device *dev);

/*
 * Object for iterating through the cache.
 */
struct dev_iter;
struct dev_iter *dev_iter_create(struct dev_filter *f, int unused);
void dev_iter_destroy(struct dev_iter *iter);
struct device *dev_iter_get(struct cmd_context *cmd, struct dev_iter *iter);

void dev_reset_error_count(struct cmd_context *cmd);

void dev_cache_failed_path(struct device *dev, const char *path);

#endif
