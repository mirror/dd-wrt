/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

// @file    shmem_common.h
// @author  Pramod Chandrashekar <pramod@sourcefire.com>

#ifndef _SHMEMCOMMON_H_
#define _SHMEMCOMMON_H_
#include "sf_types.h"
#include "snort_debug.h"
#include "../reputation_debug.h"

#define IPREP           0

#define UNKNOWN_LIST    0
#define MONITOR_LIST    1
#define BLACK_LIST      2
#define WHITE_LIST      3

#define VERSION_FILENAME "IPRVersion.dat"
#define MANIFEST_FILENAME "zone.info"

#endif
