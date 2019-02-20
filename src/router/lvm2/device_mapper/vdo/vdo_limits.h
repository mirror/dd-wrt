/*
 * Copyright (C) 2018 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef DEVICE_MAPPER_VDO_LIMITS_H
#define DEVICE_MAPPER_VDO_LIMITS_H

#define DM_VDO_BLOCK_SIZE			UINT64_C(8)		// 4KiB in sectors

#define DM_VDO_BLOCK_MAP_CACHE_SIZE_MINIMUM_MB	(128)			// 128MiB
#define DM_VDO_BLOCK_MAP_CACHE_SIZE_MAXIMUM_MB	(16 * 1024 * 1024 - 1)	// 16TiB - 1
#define DM_VDO_BLOCK_MAP_CACHE_SIZE_MINIMUM_PER_LOGICAL_THREAD  (4096 * DM_VDO_BLOCK_SIZE_KB)

#define DM_VDO_BLOCK_MAP_PERIOD_MINIMUM		1
#define DM_VDO_BLOCK_MAP_PERIOD_MAXIMUM		(16380)

#define DM_VDO_INDEX_MEMORY_SIZE_MINIMUM_MB	(256)			// 0.25 GiB
#define DM_VDO_INDEX_MEMORY_SIZE_MAXIMUM_MB	(1024 * 1024 * 1024)	// 1TiB

//#define DM_VDO_READ_CACHE_SIZE_MINIMUM_MB	(0)
#define DM_VDO_READ_CACHE_SIZE_MAXIMUM_MB	(16 * 1024 * 1024 - 1)	// 16TiB - 1

#define DM_VDO_SLAB_SIZE_MINIMUM_MB		(128)			// 128MiB
#define DM_VDO_SLAB_SIZE_MAXIMUM_MB		(32 * 1024)		// 32GiB

//#define DM_VDO_LOGICAL_SIZE_MINIMUM_MB	(0)
#define DM_VDO_LOGICAL_SIZE_MAXIMUM_MB	(UINT64_C(4) * 1024 * 1024 * 1024) // 4PiB

//#define DM_VDO_ACK_THREADS_MINIMUM		(0)
#define DM_VDO_ACK_THREADS_MAXIMUM		(100)

#define DM_VDO_BIO_THREADS_MINIMUM		(1)
#define DM_VDO_BIO_THREADS_MAXIMUM		(100)

#define DM_VDO_BIO_ROTATION_MINIMUM		(1)
#define DM_VDO_BIO_ROTATION_MAXIMUM		(1024)

#define DM_VDO_CPU_THREADS_MINIMUM		(1)
#define DM_VDO_CPU_THREADS_MAXIMUM		(100)

//#define DM_VDO_HASH_ZONE_THREADS_MINIMUM	(0)
#define DM_VDO_HASH_ZONE_THREADS_MAXIMUM	(100)

//#define DM_VDO_LOGICAL_THREADS_MINIMUM	(0)
#define DM_VDO_LOGICAL_THREADS_MAXIMUM		(100)

//#define DM_VDO_PHYSICAL_THREADS_MINIMUM	(0)
#define DM_VDO_PHYSICAL_THREADS_MAXIMUM		(16)

#endif // DEVICE_MAPPER_VDO_LIMITS_H
