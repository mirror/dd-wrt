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

#ifndef _LVM_FS_H
#define _LVM_FS_H

#include "lib/metadata/metadata.h"

/*
 * These calls, private to the activate unit, set
 * up the volume group directory in /dev and the
 * symbolic links to the dm device.
 */
int fs_add_lv(const struct logical_volume *lv, const char *dev);
int fs_del_lv(const struct logical_volume *lv);
int fs_del_lv_byname(const char *dev_dir, const char *vg_name,
		     const char *lv_name, int check_udev);
int fs_rename_lv(const struct logical_volume *lv, const char *dev,
		 const char *old_vgname, const char *old_lvname);
/* void fs_unlock(void);  moved to activate.h */
uint32_t fs_get_cookie(void);
void fs_set_cookie(uint32_t cookie);
void fs_set_create(void);
int fs_has_non_delete_ops(void);

#endif
