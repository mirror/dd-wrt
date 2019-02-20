/*
 * Copyright (C) 2010 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_PERCENT_H
#define _LVM_PERCENT_H
#include <stdint.h>

typedef enum {
	SIGN_NONE = 0,
	SIGN_PLUS = 1,
	SIGN_MINUS = 2
} sign_t;

typedef enum {
	PERCENT_NONE = 0,
	PERCENT_VG,
	PERCENT_FREE,
	PERCENT_LV,
	PERCENT_PVS,
	PERCENT_ORIGIN
} percent_type_t;

#define LVM_PERCENT_MERGE_FAILED DM_PERCENT_FAILED

uint32_t percent_of_extents(uint32_t percents, uint32_t count, int roundup);

#endif
