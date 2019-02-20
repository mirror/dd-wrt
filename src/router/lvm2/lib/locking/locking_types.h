/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2009 Red Hat, Inc. All rights reserved.
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

#include "lib/metadata/metadata.h"
#include "lib/config/config.h"

typedef int (*lock_resource_fn) (struct cmd_context * cmd, const char *resource,
				 uint32_t flags, const struct logical_volume *lv);
typedef int (*query_resource_fn) (const char *resource, const char *node, int *mode);

typedef void (*fin_lock_fn) (void);
typedef void (*reset_lock_fn) (void);

#define LCK_FLOCK			0x00000001

struct locking_type {
	uint32_t flags;   /* 0 means file locking is disabled */
	lock_resource_fn lock_resource;
	query_resource_fn query_resource;

	reset_lock_fn reset_locking;
	fin_lock_fn fin_locking;
};

int init_file_locking(struct locking_type *locking, struct cmd_context *cmd,
		      int suppress_messages);
