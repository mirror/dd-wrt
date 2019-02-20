/*
 * Copyright (C) 2015 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_PVMOVE_H
#define _LVM_PVMOVE_H

struct cmd_context;
struct dm_list;
struct logical_volume;
struct volume_group;

int pvmove_update_metadata(struct cmd_context *cmd, struct volume_group *vg,
			   struct logical_volume *lv_mirr,
			   struct dm_list *lvs_changed, unsigned flags);

int pvmove_finish(struct cmd_context *cmd, struct volume_group *vg,
		  struct logical_volume *lv_mirr, struct dm_list *lvs_changed);

#endif  /* _LVM_PVMOVE_H */
