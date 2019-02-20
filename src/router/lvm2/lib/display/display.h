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

#ifndef _LVM_DISPLAY_H
#define _LVM_DISPLAY_H

#include "lib/metadata/metadata-exported.h"
#include "lib/locking/locking.h"
#include "lib/misc/lvm-string.h"

#include <stdint.h>

const char *display_lvname(const struct logical_volume *lv);

const char *display_percent(struct cmd_context *cmd, dm_percent_t percent);

/* Specify size in KB */
const char *display_size(const struct cmd_context *cmd, uint64_t size);
const char *display_size_long(const struct cmd_context *cmd, uint64_t size);
const char *display_size_units(const struct cmd_context *cmd, uint64_t size);

char *display_uuid(char *uuidstr);
void display_stripe(const struct lv_segment *seg, uint32_t s, const char *pre);

void pvdisplay_colons(const struct physical_volume *pv);
void pvdisplay_segments(const struct physical_volume *pv);
void pvdisplay_full(const struct cmd_context *cmd,
		    const struct physical_volume *pv,
		    void *handle);
int pvdisplay_short(const struct cmd_context *cmd,
		    const struct volume_group *vg,
		    const struct physical_volume *pv, void *handle);

void lvdisplay_colons(const struct logical_volume *lv);
int lvdisplay_segments(const struct logical_volume *lv);
int lvdisplay_full(struct cmd_context *cmd, const struct logical_volume *lv,
		   void *handle);

void vgdisplay_extents(const struct volume_group *vg);
void vgdisplay_full(const struct volume_group *vg);
void vgdisplay_colons(const struct volume_group *vg);
void vgdisplay_short(const struct volume_group *vg);

void display_formats(const struct cmd_context *cmd);
void display_segtypes(const struct cmd_context *cmd);
void display_tags(const struct cmd_context *cmd);

void display_name_error(name_error_t name_error);

/*
 * Allocation policy display conversion routines.
 */
const char *get_alloc_string(alloc_policy_t alloc);
char alloc_policy_char(alloc_policy_t alloc);
alloc_policy_t get_alloc_from_string(const char *str);

const char *get_lock_type_string(lock_type_t lock_type);
lock_type_t get_lock_type_from_string(const char *str);

const char *get_percent_string(percent_type_t def);

char yes_no_prompt(const char *prompt, ...) __attribute__ ((format(printf, 1, 2)));

#endif
