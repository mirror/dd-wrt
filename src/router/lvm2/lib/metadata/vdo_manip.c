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

#include "lib/misc/lib.h"
#include "lib/metadata/metadata.h"
#include "lib/locking/locking.h"
#include "lib/misc/lvm-string.h"
#include "lib/commands/toolcontext.h"
#include "lib/display/display.h"
#include "lib/metadata/segtype.h"
#include "lib/activate/activate.h"
#include "lib/config/defaults.h"
#include "lib/misc/lvm-exec.h"

const char *get_vdo_compression_state_name(enum dm_vdo_compression_state state)
{
	switch (state) {
	case DM_VDO_COMPRESSION_ONLINE:
		return "online";
	default:
		log_debug(INTERNAL_ERROR "Unrecognized compression state: %u.", state);
		/* Fall through */
	case DM_VDO_COMPRESSION_OFFLINE:
		return "offline";
	}
}

const char *get_vdo_index_state_name(enum dm_vdo_index_state state)
{
	switch (state) {
	case DM_VDO_INDEX_ERROR:
		return "error";
	case DM_VDO_INDEX_CLOSED:
		return "closed";
	case DM_VDO_INDEX_OPENING:
		return "opening";
	case DM_VDO_INDEX_CLOSING:
		return "closing";
	case DM_VDO_INDEX_OFFLINE:
		return "offline";
	case DM_VDO_INDEX_ONLINE:
		return "online";
	default:
		log_debug(INTERNAL_ERROR "Unrecognized index state: %u.", state);
		/* Fall through */
	case DM_VDO_INDEX_UNKNOWN:
		return "unknown";
	}
}

const char *get_vdo_operating_mode_name(enum dm_vdo_operating_mode mode)
{
	switch (mode) {
	case DM_VDO_MODE_RECOVERING:
		return "recovering";
	case DM_VDO_MODE_READ_ONLY:
		return "read-only";
	default:
		log_debug(INTERNAL_ERROR "Unrecognized operating mode: %u.", mode);
		/* Fall through */
	case DM_VDO_MODE_NORMAL:
		return "normal";
	}
}

/*
 * Size of VDO virtual LV is adding header_size in front and back of device
 * to avoid colission with blkid checks.
 */
static uint64_t _get_virtual_size(uint32_t extents, uint32_t extent_size,
				  uint32_t header_size)
{
	return (uint64_t) extents * extent_size + 2 * header_size;
}

uint64_t get_vdo_pool_virtual_size(const struct lv_segment *vdo_pool_seg)
{
	return _get_virtual_size(vdo_pool_seg->vdo_pool_virtual_extents,
				 vdo_pool_seg->lv->vg->extent_size,
				 vdo_pool_seg->vdo_pool_header_size);
}

static int _sysfs_get_kvdo_value(const char *dm_name, const char *vdo_param, uint64_t *value)
{
	char path[PATH_MAX];
	char temp[64];
	int fd, size, r = 0;

	if (dm_snprintf(path, sizeof(path), "%skvdo/%s/%s",
			dm_sysfs_dir(), dm_name, vdo_param) < 0) {
		log_error("Failed to build kmod path.");
		return 0;
	}

	if ((fd = open(path, O_RDONLY)) < 0) {
		if (errno != ENOENT)
			log_sys_error("open", path);
		else
			log_sys_debug("open", path);
		goto bad;
	}

	if ((size = read(fd, temp, sizeof(temp) - 1)) < 0) {
		log_sys_error("read", path);
		goto bad;
	}
	temp[size] = 0;
	errno = 0;
	*value = strtoll(temp, NULL, 0);
	if (errno) {
		log_sys_error("strtool", path);
		goto bad;
	}

	r = 1;
bad:
	if (fd >= 0 && close(fd))
		log_sys_error("close", path);

	return r;
}

int parse_vdo_pool_status(struct dm_pool *mem, const struct logical_volume *vdo_pool_lv,
			  const char *params, struct lv_status_vdo *status)
{
	struct dm_vdo_status_parse_result result;
	char *dm_name;

	status->usage = DM_PERCENT_INVALID;
	status->saving = DM_PERCENT_INVALID;
	status->data_usage = DM_PERCENT_INVALID;

	if (!(dm_name = dm_build_dm_name(mem, vdo_pool_lv->vg->name,
					 vdo_pool_lv->name, NULL))) {
		log_error("Failed to build VDO DM name %s.",
			  display_lvname(vdo_pool_lv));
		return 0;
	}

	if (!dm_vdo_status_parse(mem, params, &result)) {
		log_error("Cannot parse %s VDO pool status %s.",
			  display_lvname(vdo_pool_lv), result.error);
		return 0;
	}

	status->vdo = result.status;

	if (result.status->operating_mode == DM_VDO_MODE_NORMAL) {
		if (!_sysfs_get_kvdo_value(dm_name, "statistics/data_blocks_used",
					   &status->data_blocks_used))
			return_0;

		if (!_sysfs_get_kvdo_value(dm_name, "statistics/logical_blocks_used",
					   &status->logical_blocks_used))
			return_0;

		status->usage = dm_make_percent(result.status->used_blocks,
						result.status->total_blocks);
		status->saving = dm_make_percent(status->logical_blocks_used - status->data_blocks_used,
						 status->logical_blocks_used);
		status->data_usage = dm_make_percent(status->data_blocks_used * DM_VDO_BLOCK_SIZE,
						     first_seg(vdo_pool_lv)->vdo_pool_virtual_extents *
						     (uint64_t) vdo_pool_lv->vg->extent_size);
	}

	return 1;
}


/*
 * Formats data LV for a use as a VDO pool LV.
 *
 * Calls tool 'vdoformat' on the already active volume.
 */
static int _format_vdo_pool_data_lv(struct logical_volume *data_lv,
				    const struct dm_vdo_target_params *vtp,
				    uint64_t *logical_size)
{
	char *dpath;
	const struct dm_config_node *cn;
	const struct dm_config_value *cv;
	struct pipe_data pdata;
	FILE *f;
	uint64_t lb;
	unsigned slabbits;
	int args = 1;
	char buf_args[5][128];
	char buf[256]; /* buffer for short disk header (64B) */
	const char *argv[19] = { /* Max supported args */
		find_config_tree_str_allow_empty(data_lv->vg->cmd, global_vdo_format_executable_CFG, NULL)
	};

	if (!(dpath = lv_path_dup(data_lv->vg->cmd->mem, data_lv))) {
		log_error("Failed to build device path for VDO formating of data volume %s.",
			  display_lvname(data_lv));
		return 0;
	}

	if (*logical_size) {
		if (dm_snprintf(buf_args[args], sizeof(buf_args[0]), "--logical-size=" FMTu64 "K",
			       (*logical_size / 2)) < 0)
			return_0;

		argv[args] = buf_args[args];
		args++;
	}

	slabbits = 31 - clz(vtp->slab_size_mb / DM_VDO_BLOCK_SIZE * 512);
	log_debug("Slab size %s converted to %u bits.",
		  display_size(data_lv->vg->cmd, vtp->slab_size_mb * UINT64_C(2 * 1024)), slabbits);
	if (dm_snprintf(buf_args[args], sizeof(buf_args[0]), "--slab-bits=%u", slabbits) < 0)
		return_0;

	argv[args] = buf_args[args];
	args++;

	if (vtp->check_point_frequency) {
		if (dm_snprintf(buf_args[args], sizeof(buf_args[0]), "--uds-checkpoint-frequency=%u",
				vtp->check_point_frequency) < 0)
			return_0;
		argv[args] = buf_args[args];
		args++;
	}

	/* Convert size to GiB units or one of these strings: 0.25, 0.50, 0.75 */
	if (vtp->index_memory_size_mb >= 1024) {
		if (dm_snprintf(buf_args[args], sizeof(buf_args[0]), "--uds-memory-size=%u",
				vtp->index_memory_size_mb / 1024) < 0)
			return_0;
	} else if (dm_snprintf(buf_args[args], sizeof(buf_args[0]), "--uds-memory-size=0.%u",
			       (vtp->index_memory_size_mb < 512) ? 25 :
			       (vtp->index_memory_size_mb < 768) ? 50 : 75) < 0)
		   return_0;

	argv[args] = buf_args[args];
	args++;

	if (vtp->use_sparse_index)  {
		if (dm_snprintf(buf_args[args], sizeof(buf_args[0]), "--uds-sparse") < 0)
			return_0;

		argv[args] = buf_args[args];
		args++;
	}

	/* Any other user opts add here */
	if (!(cn = find_config_tree_array(data_lv->vg->cmd, global_vdo_format_options_CFG, NULL))) {
		log_error(INTERNAL_ERROR "Unable to find configuration for vdoformat command options.");
		return 0;
	}

	for (cv = cn->v; cv && args < 16; cv = cv->next) {
		if (cv->type != DM_CFG_STRING) {
			log_error("Invalid string in config file: "
				  "global/vdoformat_options.");
			return 0;
		}
		if (cv->v.str[0])
			argv[++args] = cv->v.str;
	}

	/* Only unused VDO data LV could be activated and wiped */
	if (!dm_list_empty(&data_lv->segs_using_this_lv)) {
		log_error(INTERNAL_ERROR "Failed to wipe logical VDO data for volume %s.",
			  display_lvname(data_lv));
		return 0;
	}

	argv[args] = dpath;

	if (!(f = pipe_open(data_lv->vg->cmd, argv, 0, &pdata))) {
		log_error("WARNING: Cannot read output from %s.", argv[0]);
		return 0;
	}

	if (!*logical_size)
		while (fgets(buf, sizeof(buf), f)) {
			/* TODO: Watch out for locales */
			if (sscanf(buf, "Logical blocks defaulted to " FMTu64 " blocks", &lb) == 1) {
				*logical_size = lb * DM_VDO_BLOCK_SIZE;
				log_verbose("Available VDO logical blocks " FMTu64 " (%s).",
					    lb, display_size(data_lv->vg->cmd, *logical_size));
				break;
			} else
				log_warn("WARNING: Cannot parse output '%s' from %s.", buf, argv[0]);
		}

	if (!pipe_close(&pdata)) {
		log_error("Command %s failed.", argv[0]);
		return 0;
	}

	return 1;
}

/*
 * convert_vdo_pool_lv
 * @data_lv
 * @vtp
 * @virtual_extents
 *
 * Convert given data LV and its target parameters into a VDO LV with VDO pool.
 *
 * Returns: old data LV on success (passed data LV becomes VDO LV), NULL on failure
 */
struct logical_volume *convert_vdo_pool_lv(struct logical_volume *data_lv,
					   const struct dm_vdo_target_params *vtp,
					   uint32_t *virtual_extents)
{
	const uint64_t header_size = DEFAULT_VDO_POOL_HEADER_SIZE;
	const uint32_t extent_size = data_lv->vg->extent_size;
	struct cmd_context *cmd = data_lv->vg->cmd;
	struct logical_volume *vdo_pool_lv = data_lv;
	const struct segment_type *vdo_pool_segtype;
	struct lv_segment *vdo_pool_seg;
	uint64_t vdo_logical_size = 0;
	uint64_t adjust;

	if (!(vdo_pool_segtype = get_segtype_from_string(cmd, SEG_TYPE_NAME_VDO_POOL)))
		return_NULL;

	adjust = (*virtual_extents * (uint64_t) extent_size) % DM_VDO_BLOCK_SIZE;
	if (adjust) {
		*virtual_extents += (DM_VDO_BLOCK_SIZE - adjust) / extent_size;
		log_print_unless_silent("Rounding size up to 4,00 KiB VDO logical extent boundary: %s.",
					display_size(data_lv->vg->cmd, *virtual_extents * (uint64_t) extent_size));
	}

	if (*virtual_extents)
		vdo_logical_size =
			_get_virtual_size(*virtual_extents, extent_size, header_size);

	if (!dm_vdo_validate_target_params(vtp, vdo_logical_size))
		return_0;

	/* Format data LV as VDO volume */
	if (!_format_vdo_pool_data_lv(data_lv, vtp, &vdo_logical_size)) {
		log_error("Cannot format VDO pool volume %s.", display_lvname(data_lv));
		return NULL;
	}

	if (!deactivate_lv(data_lv->vg->cmd, data_lv)) {
		log_error("Aborting. Manual intervention required.");
		return NULL;
	}

	vdo_logical_size -= 2 * header_size;

	if (vdo_logical_size < extent_size) {
		if (!*virtual_extents)
			/* User has not specified size and at least 1 extent is necessary */
			log_error("Cannot create fully fitting VDO volume, "
				  "--virtualsize has to be specified.");

		log_error("Size %s for VDO volume cannot be smaller then extent size %s.",
			  display_size(data_lv->vg->cmd, vdo_logical_size),
			  display_size(data_lv->vg->cmd, extent_size));
		return NULL;
	}

	*virtual_extents = vdo_logical_size / extent_size;

	/* Move segments from existing data_lv into LV_vdata */
	if (!(data_lv = insert_layer_for_lv(cmd, vdo_pool_lv, 0, "_vdata")))
		return_NULL;

	vdo_pool_seg = first_seg(vdo_pool_lv);
	vdo_pool_seg->segtype = vdo_pool_segtype;
	vdo_pool_seg->vdo_params = *vtp;
	vdo_pool_seg->vdo_pool_header_size = DEFAULT_VDO_POOL_HEADER_SIZE;
	vdo_pool_seg->vdo_pool_virtual_extents = *virtual_extents;

	vdo_pool_lv->status |= LV_VDO_POOL;
	data_lv->status |= LV_VDO_POOL_DATA;

	return data_lv;
}

int get_vdo_write_policy(enum dm_vdo_write_policy *vwp, const char *policy)
{
	if (strcasecmp(policy, "sync") == 0)
		*vwp = DM_VDO_WRITE_POLICY_SYNC;
	else if (strcasecmp(policy, "async") == 0)
		*vwp = DM_VDO_WRITE_POLICY_ASYNC;
	else if (strcasecmp(policy, "auto") == 0)
		*vwp = DM_VDO_WRITE_POLICY_AUTO;
	else {
		log_error("Unknown VDO write policy %s.", policy);
		return 0;
	}

	return 1;
}

int fill_vdo_target_params(struct cmd_context *cmd,
			   struct dm_vdo_target_params *vtp,
			   struct profile *profile)
{
	const char *policy;

	// TODO: Postpone filling data to the moment when VG is known with profile.
	// TODO: Maybe add more lvm cmdline switches to set profile settings.

	vtp->use_compression =
		find_config_tree_int(cmd, allocation_vdo_use_compression_CFG, profile);
	vtp->use_deduplication =
		find_config_tree_int(cmd, allocation_vdo_use_deduplication_CFG, profile);
	vtp->emulate_512_sectors =
		find_config_tree_int(cmd, allocation_vdo_emulate_512_sectors_CFG, profile);
	vtp->block_map_cache_size_mb =
		find_config_tree_int64(cmd, allocation_vdo_block_map_cache_size_mb_CFG, profile);
	vtp->block_map_period =
		find_config_tree_int(cmd, allocation_vdo_block_map_period_CFG, profile);
	vtp->check_point_frequency =
		find_config_tree_int(cmd, allocation_vdo_check_point_frequency_CFG, profile);
	vtp->use_sparse_index =
		find_config_tree_int(cmd, allocation_vdo_use_sparse_index_CFG, profile);
	vtp->index_memory_size_mb =
		find_config_tree_int64(cmd, allocation_vdo_index_memory_size_mb_CFG, profile);
	vtp->use_read_cache =
		find_config_tree_int(cmd, allocation_vdo_use_read_cache_CFG, profile);
	vtp->read_cache_size_mb =
		find_config_tree_int64(cmd, allocation_vdo_read_cache_size_mb_CFG, profile);
	vtp->slab_size_mb =
		find_config_tree_int(cmd, allocation_vdo_slab_size_mb_CFG, profile);
	vtp->ack_threads =
		find_config_tree_int(cmd, allocation_vdo_ack_threads_CFG, profile);
	vtp->bio_threads =
		find_config_tree_int(cmd, allocation_vdo_bio_threads_CFG, profile);
	vtp->bio_rotation =
		find_config_tree_int(cmd, allocation_vdo_bio_rotation_CFG, profile);
	vtp->cpu_threads =
		find_config_tree_int(cmd, allocation_vdo_cpu_threads_CFG, profile);
	vtp->hash_zone_threads =
		find_config_tree_int(cmd, allocation_vdo_hash_zone_threads_CFG, profile);
	vtp->logical_threads =
		find_config_tree_int(cmd, allocation_vdo_logical_threads_CFG, profile);
	vtp->physical_threads =
		find_config_tree_int(cmd, allocation_vdo_physical_threads_CFG, profile);

	policy = find_config_tree_str(cmd, allocation_vdo_write_policy_CFG, profile);
	if (!get_vdo_write_policy(&vtp->write_policy, policy))
		return_0;

	return 1;
}
