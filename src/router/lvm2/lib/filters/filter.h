/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004 Luca Berra
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

#ifndef _LVM_FILTER_H
#define _LVM_FILTER_H

#include "lib/device/dev-cache.h"
#include "lib/device/dev-type.h"

struct dev_filter *composite_filter_create(int n, int use_dev_ext_info, struct dev_filter **filters);

struct dev_filter *lvm_type_filter_create(struct dev_types *dt);
struct dev_filter *md_filter_create(struct cmd_context *cmd, struct dev_types *dt);
struct dev_filter *fwraid_filter_create(struct dev_types *dt);
struct dev_filter *mpath_filter_create(struct dev_types *dt);
struct dev_filter *partitioned_filter_create(struct dev_types *dt);
struct dev_filter *persistent_filter_create(struct dev_types *dt, struct dev_filter *f);
struct dev_filter *sysfs_filter_create(void);
struct dev_filter *signature_filter_create(struct dev_types *dt);

struct dev_filter *internal_filter_create(void);
int internal_filter_allow(struct dm_pool *mem, struct device *dev);
void internal_filter_clear(void);

/*
 * patterns must be an array of strings of the form:
 * [ra]<sep><regex><sep>, eg,
 * r/cdrom/          - reject cdroms
 * a|loop/[0-4]|     - accept loops 0 to 4
 * r|.*|             - reject everything else
 */

struct dev_filter *regex_filter_create(const struct dm_config_value *patterns);

typedef enum {
	FILTER_MODE_NO_LVMETAD,
	FILTER_MODE_PRE_LVMETAD,
	FILTER_MODE_POST_LVMETAD
} filter_mode_t;
struct dev_filter *usable_filter_create(struct cmd_context *cmd, struct dev_types *dt, filter_mode_t mode);

#endif 	/* _LVM_FILTER_H */
