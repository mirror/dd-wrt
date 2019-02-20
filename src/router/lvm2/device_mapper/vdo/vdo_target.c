/*
 * Copyright (C) 2018 Red Hat, Inc. All rights reserved.
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

#include "device_mapper/misc/dmlib.h"
#include "device_mapper/all.h"

#include "vdo_limits.h"
#include "target.h"

bool dm_vdo_validate_target_params(const struct dm_vdo_target_params *vtp,
				   uint64_t vdo_size)
{
	bool valid = true;

	if ((vtp->block_map_cache_size_mb < DM_VDO_BLOCK_MAP_CACHE_SIZE_MINIMUM_MB) ||
	    (vtp->block_map_cache_size_mb > DM_VDO_BLOCK_MAP_CACHE_SIZE_MAXIMUM_MB)) {
		log_error("VDO block map cache size %u out of range.",
			  vtp->block_map_cache_size_mb);
		valid = false;
	}

	if ((vtp->index_memory_size_mb < DM_VDO_INDEX_MEMORY_SIZE_MINIMUM_MB) ||
	    (vtp->index_memory_size_mb > DM_VDO_INDEX_MEMORY_SIZE_MAXIMUM_MB)) {
		log_error("VDO index memory size %u out of range.",
			  vtp->index_memory_size_mb);
		valid = false;
	}

	if (vtp->read_cache_size_mb > DM_VDO_READ_CACHE_SIZE_MAXIMUM_MB) {
		log_error("VDO read cache size %u out of range.",
			  vtp->read_cache_size_mb);
		valid = false;
	}

	if ((vtp->slab_size_mb < DM_VDO_SLAB_SIZE_MINIMUM_MB) ||
	    (vtp->slab_size_mb > DM_VDO_SLAB_SIZE_MAXIMUM_MB)) {
		log_error("VDO slab size %u out of range.",
			  vtp->slab_size_mb);
		valid = false;
	}

	if (vtp->ack_threads > DM_VDO_ACK_THREADS_MAXIMUM) {
		log_error("VDO ack threads %u out of range.", vtp->ack_threads);
		valid = false;
	}

	if ((vtp->bio_threads < DM_VDO_BIO_THREADS_MINIMUM) ||
	    (vtp->bio_threads > DM_VDO_BIO_THREADS_MAXIMUM)) {
		log_error("VDO bio threads %u out of range.", vtp->bio_threads);
		valid = false;
	}

	if ((vtp->bio_rotation < DM_VDO_BIO_ROTATION_MINIMUM) ||
	    (vtp->bio_rotation > DM_VDO_BIO_ROTATION_MAXIMUM)) {
		log_error("VDO bio rotation %u out of range.", vtp->bio_rotation);
		valid = false;
	}

	if ((vtp->cpu_threads < DM_VDO_CPU_THREADS_MINIMUM) ||
	    (vtp->cpu_threads > DM_VDO_CPU_THREADS_MAXIMUM)) {
		log_error("VDO cpu threads %u out of range.", vtp->cpu_threads);
		valid = false;
	}

	if (vtp->hash_zone_threads > DM_VDO_HASH_ZONE_THREADS_MAXIMUM) {
		log_error("VDO hash zone threads %u out of range.", vtp->hash_zone_threads);
		valid = false;
	}

	if (vtp->logical_threads > DM_VDO_LOGICAL_THREADS_MAXIMUM) {
		log_error("VDO logical threads %u out of range.", vtp->logical_threads);
		valid = false;
	}

	if (vtp->physical_threads > DM_VDO_PHYSICAL_THREADS_MAXIMUM) {
		log_error("VDO physical threads %u out of range.", vtp->physical_threads);
		valid = false;
	}

	switch (vtp->write_policy) {
	case DM_VDO_WRITE_POLICY_SYNC:
	case DM_VDO_WRITE_POLICY_ASYNC:
	case DM_VDO_WRITE_POLICY_AUTO:
		break;
	default:
		log_error(INTERNAL_ERROR "VDO write policy %u is unknown.", vtp->write_policy);
		valid = false;
	}

	if ((vtp->hash_zone_threads ||
	     vtp->logical_threads ||
	     vtp->physical_threads) &&
	    (!vtp->hash_zone_threads ||
	     !vtp->logical_threads ||
	     !vtp->physical_threads)) {
		log_error("Value of vdo_hash_zone_threads(%u), vdo_logical_threads(%u), "
			  "vdo_physical_threads(%u) must be all zero or all non-zero.",
			  vtp->hash_zone_threads, vtp->logical_threads, vtp->physical_threads);
		valid = false;
	}

	if (vdo_size >= (DM_VDO_LOGICAL_SIZE_MAXIMUM_MB * UINT64_C(1024 * 2))) {
		log_error("VDO logical size is by " FMTu64 "KiB bigger then limit " FMTu64 "TiB.",
			  (vdo_size - (DM_VDO_LOGICAL_SIZE_MAXIMUM_MB * UINT64_C(1024 * 2))) / 2,
			  DM_VDO_LOGICAL_SIZE_MAXIMUM_MB / UINT64_C(1024) / UINT64_C(1024));
		valid = false;
	}

	return valid;
}
