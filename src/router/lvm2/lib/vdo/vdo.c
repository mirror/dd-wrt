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
#include "lib/activate/activate.h"
#include "lib/activate/targets.h"
#include "lib/commands/toolcontext.h"
#include "lib/datastruct/str_list.h"
#include "lib/display/display.h"
#include "lib/format_text/text_export.h"
#include "lib/log/lvm-logging.h"
#include "lib/metadata/metadata.h"
#include "lib/metadata/lv_alloc.h"
#include "lib/metadata/segtype.h"
#include "base/memory/zalloc.h"

static unsigned _feature_mask;

static int _bad_field(const char *field)
{
	log_error("Couldn't read '%s' for VDO segment.", field);
	return 0;
}

static int _import_bool(const struct dm_config_node *n,
			const char *name, bool *b)
{
	uint32_t t;

	if (dm_config_has_node(n, name)) {
		if (!dm_config_get_uint32(n, name, &t))
			return _bad_field(name);

		if (t) {
			*b = true;
			return 1;
		}
	}

	*b = false;

	return 1;
}

static void _print_yes_no(const char *name, bool value)
{
	log_print("  %s\t%s", name, value ? "yes" : "no");
}

/*
 * VDO linear mapping
 */
static const char *_vdo_name(const struct lv_segment *seg)
{
	return SEG_TYPE_NAME_VDO;
}

static void _vdo_display(const struct lv_segment *seg)
{
	display_stripe(seg, 0, "    ");
}

static int _vdo_text_import(struct lv_segment *seg,
			    const struct dm_config_node *n,
			    struct dm_hash_table *pv_hash __attribute__((unused)))
{
	struct logical_volume *vdo_pool_lv;
	const char *str;
	uint32_t vdo_offset;

	if (!dm_config_has_node(n, "vdo_pool") ||
	    !(str = dm_config_find_str(n, "vdo_pool", NULL)))
		return _bad_field("vdo_pool");
	if (!(vdo_pool_lv = find_lv(seg->lv->vg, str))) {
		log_error("Unknown VDO pool logical volume %s.", str);
		return 0;
	}

	if (!dm_config_get_uint32(n, "vdo_offset", &vdo_offset))
		return _bad_field("vdo_offset");

	if (!set_lv_segment_area_lv(seg, 0, vdo_pool_lv, vdo_offset, LV_VDO_POOL))
		return_0;

	seg->lv->status |= LV_VDO;

	return 1;
}

static int _vdo_text_export(const struct lv_segment *seg, struct formatter *f)
{

	if (!seg_is_vdo(seg)) {
		log_error(INTERNAL_ERROR "Passed segment is not VDO type.");
		return 0;
	}

	outf(f, "vdo_pool = \"%s\"", seg_lv(seg, 0)->name);
	outf(f, "vdo_offset = %u", seg_le(seg, 0));

	return 1;
}

#ifdef DEVMAPPER_SUPPORT
static int _vdo_target_status_compatible(const char *type)
{
	return (strcmp(type, TARGET_NAME_LINEAR) == 0);
}

static int _vdo_add_target_line(struct dev_manager *dm,
				struct dm_pool *mem __attribute__((unused)),
				struct cmd_context *cmd,
				void **target_state __attribute__((unused)),
				struct lv_segment *seg,
				const struct lv_activate_opts *laopts __attribute__((unused)),
				struct dm_tree_node *node, uint64_t len,
				uint32_t *pvmove_mirror_count __attribute__((unused)))
{
	char *vdo_pool_uuid;

	if (!(vdo_pool_uuid = build_dm_uuid(mem, seg_lv(seg, 0), lv_layer(seg_lv(seg, 0)))))
		return_0;

	if (!add_linear_area_to_dtree(node, len, seg->lv->vg->extent_size,
				      cmd->use_linear_target,
				      seg->lv->vg->name, seg->lv->name))
		return_0;

	if (!dm_tree_node_add_target_area(node, NULL, vdo_pool_uuid,
					  first_seg(seg_lv(seg, 0))->vdo_pool_header_size +
					  seg->lv->vg->extent_size * (uint64_t)seg_le(seg, 0)))
		return_0;

	return 1;
}

#endif

/*
 *  VDO pool
 */
static const char *_vdo_pool_name(const struct lv_segment *seg)
{
	return SEG_TYPE_NAME_VDO_POOL;
}

static void _vdo_pool_display(const struct lv_segment *seg)
{
	struct cmd_context *cmd = seg->lv->vg->cmd;
	const struct dm_vdo_target_params *vtp = &seg->vdo_params;

	log_print("  Virtual size\t%s", display_size(cmd, get_vdo_pool_virtual_size(seg)));
	log_print("  Header size\t\t%s", display_size(cmd, seg->vdo_pool_header_size));

	_print_yes_no("Compression\t", vtp->use_compression);
	_print_yes_no("Deduplication", vtp->use_deduplication);
	_print_yes_no("Emulate 512 sectors", vtp->emulate_512_sectors);

	log_print("  Block map cache sz\t%s",
		  display_size(cmd, vtp->block_map_cache_size_mb * UINT64_C(2 * 1024)));
	log_print("  Block map period\t%u", vtp->block_map_period);

	_print_yes_no("Sparse index", vtp->use_sparse_index);

	log_print("  Index memory size\t%s",
		  display_size(cmd, vtp->index_memory_size_mb * UINT64_C(2 * 1024)));

	_print_yes_no("Using read cache", vtp->use_read_cache);

	log_print("  Read cache size\t%s",
		  display_size(cmd, vtp->read_cache_size_mb * UINT64_C(2 * 1024)));
	log_print("  Slab size\t\t%s",
		  display_size(cmd, vtp->slab_size_mb * UINT64_C(2 * 1024)));

	log_print("  # Ack threads\t%u", (unsigned) vtp->ack_threads);
	log_print("  # Bio threads\t%u", (unsigned) vtp->bio_threads);
	log_print("  Bio rotation\t%u", (unsigned) vtp->bio_rotation);
	log_print("  # CPU threads\t%u", (unsigned) vtp->cpu_threads);
	log_print("  # Hash zone threads\t%u", (unsigned) vtp->hash_zone_threads);
	log_print("  # Logical threads\t%u", (unsigned) vtp->logical_threads);
	log_print("  # Physical threads\t%u", (unsigned) vtp->physical_threads);
}

/* reused as _vdo_text_import_area_count */
static int _vdo_pool_text_import_area_count(const struct dm_config_node *sn __attribute__((unused)),
					    uint32_t *area_count)
{
	*area_count = 1;

	return 1;
}

static int _vdo_pool_text_import(struct lv_segment *seg,
				 const struct dm_config_node *n,
				 struct dm_hash_table *pv_hash __attribute__((unused)))
{
	struct dm_vdo_target_params *vtp = &seg->vdo_params;
	struct logical_volume *data_lv;
	const char *str;

	if (!dm_config_has_node(n, "data") ||
	    !(str = dm_config_find_str(n, "data", NULL)))
		return _bad_field("data");
	if (!(data_lv = find_lv(seg->lv->vg, str))) {
		log_error("Unknown logical volume %s.", str);
		return 0;
	}

	/*
	 * TODO: we may avoid printing settings with FIXED default values
	 *       so it would generate smaller metadata.
	 */
	if (!dm_config_get_uint32(n, "header_size", &seg->vdo_pool_header_size))
		return _bad_field("header_size");

	if (!dm_config_get_uint32(n, "virtual_extents", &seg->vdo_pool_virtual_extents))
		return _bad_field("virtual_extents");

	memset(vtp, 0, sizeof(*vtp));

	if (!_import_bool(n, "use_compression", &vtp->use_compression))
		return_0;

	if (!_import_bool(n, "use_deduplication", &vtp->use_deduplication))
		return_0;

	if (!_import_bool(n, "emulate_512_sectors", &vtp->emulate_512_sectors))
		return_0;

	if (!dm_config_get_uint32(n, "block_map_cache_size_mb", &vtp->block_map_cache_size_mb))
		return _bad_field("block_map_cache_size_mb");

	if (!dm_config_get_uint32(n, "block_map_period", &vtp->block_map_period))
		return _bad_field("block_map_period");

	if (!_import_bool(n, "use_sparse_index", &vtp->use_sparse_index))
		return_0;

	if (!dm_config_get_uint32(n, "index_memory_size_mb", &vtp->index_memory_size_mb))
		return _bad_field("index_memory_size_mb");

	if (!_import_bool(n, "use_read_cache", &vtp->use_read_cache))
		return_0;

	if (!dm_config_get_uint32(n, "read_cache_size_mb", &vtp->read_cache_size_mb))
		return _bad_field("read_cache_size_mb");

	if (!dm_config_get_uint32(n, "slab_size_mb", &vtp->slab_size_mb))
		return _bad_field("slab_size_mb");

	if (!dm_config_get_uint32(n, "ack_threads", &vtp->ack_threads))
		return _bad_field("ack_threads");

	if (!dm_config_get_uint32(n, "bio_threads", &vtp->bio_threads))
		return _bad_field("bio_threads");

	if (!dm_config_get_uint32(n, "bio_rotation", &vtp->bio_rotation))
		return _bad_field("bio_rotation");

	if (!dm_config_get_uint32(n, "cpu_threads", &vtp->cpu_threads))
		return _bad_field("cpu_threads");

	if (!dm_config_get_uint32(n, "hash_zone_threads", &vtp->hash_zone_threads))
		return _bad_field("hash_zone_threads");

	if (!dm_config_get_uint32(n, "logical_threads", &vtp->logical_threads))
		return _bad_field("logical_threads");

	if (!dm_config_get_uint32(n, "physical_threads", &vtp->physical_threads))
		return _bad_field("physical_threads");


	if (!set_lv_segment_area_lv(seg, 0, data_lv, 0, LV_VDO_POOL_DATA))
		return_0;

	seg->lv->status |= LV_VDO_POOL;
	lv_set_hidden(data_lv);

	return 1;
}

static int _vdo_pool_text_export(const struct lv_segment *seg, struct formatter *f)
{
	const struct dm_vdo_target_params *vtp = &seg->vdo_params;

	outf(f, "data = \"%s\"", seg_lv(seg, 0)->name);
	outsize(f, seg->vdo_pool_header_size, "header_size = %u\t",
		seg->vdo_pool_header_size);
	outsize(f, seg->vdo_pool_virtual_extents * (uint64_t) seg->lv->vg->extent_size,
		"virtual_extents = %u\t", seg->vdo_pool_virtual_extents);

	outnl(f);

	if (vtp->use_compression)
		outf(f, "use_compression = 1");
	if (vtp->use_deduplication)
		outf(f, "use_deduplication = 1");
	if (vtp->emulate_512_sectors)
		outf(f, "emulate_512_sectors = 1");

	outsize(f, vtp->block_map_cache_size_mb * UINT64_C(2 * 1024),
		"block_map_cache_size_mb = %u", vtp->block_map_cache_size_mb);
	outf(f, "block_map_period = %u", vtp->block_map_period);

	if (vtp->use_sparse_index)
		outf(f, "use_sparse_index = 1");
	// TODO - conditionally
	outsize(f, vtp->index_memory_size_mb * UINT64_C(2 * 1024),
		"index_memory_size_mb = %u", vtp->index_memory_size_mb);

	if (vtp->use_read_cache)
		outf(f, "use_read_cache = 1");
	// TODO - conditionally
	outsize(f, vtp->read_cache_size_mb * UINT64_C(2 * 1024),
		"read_cache_size_mb = %u", vtp->read_cache_size_mb);
	outsize(f, vtp->slab_size_mb * UINT64_C(2 * 1024),
		"slab_size_mb = %u", vtp->slab_size_mb);
	outf(f, "ack_threads = %u", (unsigned) vtp->ack_threads);
	outf(f, "bio_threads = %u", (unsigned) vtp->bio_threads);
	outf(f, "bio_rotation = %u", (unsigned) vtp->bio_rotation);
	outf(f, "cpu_threads = %u", (unsigned) vtp->cpu_threads);
	outf(f, "hash_zone_threads = %u", (unsigned) vtp->hash_zone_threads);
	outf(f, "logical_threads = %u", (unsigned) vtp->logical_threads);
	outf(f, "physical_threads = %u", (unsigned) vtp->physical_threads);

	return 1;
}

#ifdef DEVMAPPER_SUPPORT
static int _vdo_pool_target_status_compatible(const char *type)
{
	return (strcmp(type, TARGET_NAME_VDO) == 0);
}

static int _vdo_pool_add_target_line(struct dev_manager *dm,
				     struct dm_pool *mem __attribute__((unused)),
				     struct cmd_context *cmd __attribute__((unused)),
				     void **target_state __attribute__((unused)),
				     struct lv_segment *seg,
				     const struct lv_activate_opts *laopts __attribute__((unused)),
				     struct dm_tree_node *node, uint64_t len,
				     uint32_t *pvmove_mirror_count __attribute__((unused)))
{
	char *data_uuid;

	if (!seg_is_vdo_pool(seg)) {
		log_error(INTERNAL_ERROR "Passed segment is not VDO pool.");
		return 0;
	}

	if (!(data_uuid = build_dm_uuid(mem, seg_lv(seg, 0), lv_layer(seg_lv(seg, 0)))))
		return_0;

	/* VDO uses virtual size instead of its physical size */
	if (!dm_tree_node_add_vdo_target(node, get_vdo_pool_virtual_size(seg),
					 data_uuid, &seg->vdo_params))
		return_0;

	return 1;
}

static int _vdo_target_present(struct cmd_context *cmd,
			       const struct lv_segment *seg __attribute__((unused)),
			       unsigned *attributes __attribute__((unused)))
{
	/* List of features with their kernel target version */
	static const struct feature {
		uint32_t maj;
		uint32_t min;
		unsigned vdo_feature;
		const char *feature;
	} _features[] = {
		{ 1, 1, 0, "" },
		//{ 9, 9, VDO_FEATURE_RESIZE, "resize" },
	};
	//static const char _lvmconf[] = "global/vdo_disabled_features";
	static int _vdo_checked = 0;
	static int _vdo_present = 0;
	static unsigned _vdo_attrs = 0;
	uint32_t i, maj, min, patchlevel;
	const struct segment_type *segtype;

	if (!activation())
		return 0;

	if (!_vdo_checked) {
		_vdo_checked = 1;

		if (!target_present_version(cmd, TARGET_NAME_VDO, 0,
					    &maj, &min, &patchlevel)) {
			/* Try to load kmod VDO module */
			if (!module_present(cmd, MODULE_NAME_VDO) ||
			    !target_version(TARGET_NAME_VDO, &maj, &min, &patchlevel))
				return_0;
		}

		if (maj < 6 || (maj == 6 && min < 2)) {
			log_warn("WARNING: VDO target version %u.%u.%u is too old.",
				 maj, min, patchlevel);
			return 0;
		}

		/* If stripe target was already detected, reuse its result */
		if (!(segtype = get_segtype_from_string(cmd, SEG_TYPE_NAME_STRIPED)) ||
		    !segtype->ops->target_present || !segtype->ops->target_present(cmd, NULL, NULL)) {
			/* Linear/Stripe targer is for mapping LVs on top of single VDO volume. */
			if (!target_present(cmd, TARGET_NAME_LINEAR, 0) ||
			    !target_present(cmd, TARGET_NAME_STRIPED, 0))
				return 0;
		}

		_vdo_present = 1;
		/* Prepare for adding supported features */
		for (i = 0; i < DM_ARRAY_SIZE(_features); ++i)
			if ((maj > _features[i].maj) ||
			    (maj == _features[i].maj && min >= _features[i].min))
				_vdo_attrs |= _features[i].vdo_feature;
			else
				log_very_verbose("Target %s does not support %s.",
						 TARGET_NAME_VDO,
						 _features[i].feature);
	}

	if (attributes) {
		*attributes = _vdo_attrs & _feature_mask;
	}

	return _vdo_present;
}

static int _vdo_modules_needed(struct dm_pool *mem,
			   const struct lv_segment *seg __attribute__((unused)),
			   struct dm_list *modules)
{
	if (!str_list_add(mem, modules, MODULE_NAME_VDO)) {
		log_error("String list allocation failed for VDO module.");
		return 0;
	}

	return 1;
}

#  ifdef DMEVENTD
/* FIXME Cache this */
static int _vdo_pool_target_registered(struct lv_segment *seg, int *pending, int *monitored)
{
	return target_registered_with_dmeventd(seg->lv->vg->cmd,
					       seg->segtype->dso,
					       seg->lv, pending, monitored);
}

/* FIXME This gets run while suspended and performs banned operations. */
static int _vdo_pool_target_set_events(struct lv_segment *seg, int evmask, int set)
{
	/* FIXME Make timeout (10) configurable */
	return target_register_events(seg->lv->vg->cmd,
				      seg->segtype->dso,
				      seg->lv, evmask, set, 10);
}

static int _vdo_pool_target_register_events(struct lv_segment *seg,
					    int events)
{
	return _vdo_pool_target_set_events(seg, events, 1);
}

static int _vdo_pool_target_unregister_events(struct lv_segment *seg,
					      int events)
{
	return _vdo_pool_target_set_events(seg, events, 0);
}

#  endif /* DMEVENTD */
#endif

/* reused as _vdo_destroy */
static void _vdo_pool_destroy(struct segment_type *segtype)
{
	free((void *)segtype->dso);
	free((void *)segtype);
}

static struct segtype_handler _vdo_ops = {
	.name = _vdo_name,
	.display = _vdo_display,
	.text_import = _vdo_text_import,
	.text_import_area_count = _vdo_pool_text_import_area_count,
	.text_export = _vdo_text_export,

#ifdef DEVMAPPER_SUPPORT
	.target_status_compatible = _vdo_target_status_compatible,
	.add_target_line = _vdo_add_target_line,
	.target_present = _vdo_target_present,
	.modules_needed = _vdo_modules_needed,
#endif
	.destroy = _vdo_pool_destroy,
};

static struct segtype_handler _vdo_pool_ops = {
	.name = _vdo_pool_name,
	.display = _vdo_pool_display,
	.text_import = _vdo_pool_text_import,
	.text_import_area_count = _vdo_pool_text_import_area_count,
	.text_export = _vdo_pool_text_export,

#ifdef DEVMAPPER_SUPPORT
	.target_status_compatible = _vdo_pool_target_status_compatible,
	.add_target_line = _vdo_pool_add_target_line,
	.target_present = _vdo_target_present,
	.modules_needed = _vdo_modules_needed,

#  ifdef DMEVENTD
	.target_monitored = _vdo_pool_target_registered,
	.target_monitor_events = _vdo_pool_target_register_events,
	.target_unmonitor_events = _vdo_pool_target_unregister_events,
#  endif /* DMEVENTD */
#endif
	.destroy = _vdo_pool_destroy,
};

int init_vdo_segtypes(struct cmd_context *cmd,
		      struct segtype_library *seglib)
{
	struct segment_type *segtype, *pool_segtype;

	if (!(segtype = zalloc(sizeof(*segtype))) ||
	    !(pool_segtype = zalloc(sizeof(*segtype)))) {
		log_error("Failed to allocate memory for VDO segtypes.");
		free(segtype);
		return 0;
	}

	segtype->name = SEG_TYPE_NAME_VDO;
	segtype->flags = SEG_VDO | SEG_VIRTUAL | SEG_ONLY_EXCLUSIVE;
	segtype->ops = &_vdo_ops;

	if (!lvm_register_segtype(seglib, segtype)) {
		free(pool_segtype);
		return_0;
	}

	pool_segtype->name = SEG_TYPE_NAME_VDO_POOL;
	pool_segtype->flags = SEG_VDO_POOL | SEG_ONLY_EXCLUSIVE;
	pool_segtype->ops = &_vdo_pool_ops;
#ifdef DEVMAPPER_SUPPORT
#  ifdef DMEVENTD
	pool_segtype->dso = get_monitor_dso_path(cmd, dmeventd_vdo_library_CFG);
	if (pool_segtype->dso)
		pool_segtype->flags |= SEG_MONITORED;
#  endif /* DMEVENTD */
#endif

	if (!lvm_register_segtype(seglib, pool_segtype))
		return_0;

	log_very_verbose("Initialised segtypes: %s, %s.", segtype->name, pool_segtype->name);

	/* Reset mask for recalc */
	_feature_mask = 0;

	return 1;
}
