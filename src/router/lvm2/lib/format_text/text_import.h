/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2005 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_TEXT_IMPORT_H
#define _LVM_TEXT_IMPORT_H

#include <inttypes.h>

struct dm_hash_table;
struct lv_segment;
struct dm_config_node;

int text_import_areas(struct lv_segment *seg, const struct dm_config_node *sn,
		      const struct dm_config_value *cv, struct dm_hash_table *pv_hash,
		      uint64_t status);

#endif
