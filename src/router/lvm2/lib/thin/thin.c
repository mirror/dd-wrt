/*
 * Copyright (C) 2011-2013 Red Hat, Inc. All rights reserved.
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

#include "base/memory/zalloc.h"
#include "lib/misc/lib.h"
#include "lib/display/display.h"
#include "lib/metadata/metadata.h"
#include "lib/metadata/segtype.h"
#include "lib/format_text/text_export.h"
#include "lib/config/config.h"
#include "lib/activate/activate.h"
#include "lib/datastruct/str_list.h"

/* Dm kernel module name for thin provisiong */
static const char _thin_pool_module[] = "thin-pool";
static const char _thin_module[] = "thin";

/*
 * Macro used as return argument - returns 0.
 * return is left to be written in the function for better readability.
 */
#define SEG_LOG_ERROR(t, p...) \
	log_error(t " segment %s of logical volume %s.", ## p, \
		  dm_config_parent_name(sn), seg->lv->name), 0;

/* TODO: using static field here, maybe should be a part of segment_type */
static unsigned _feature_mask;

static void _thin_pool_display(const struct lv_segment *seg)
{
	log_print("  Chunk size\t\t%s",
		  display_size(seg->lv->vg->cmd, seg->chunk_size));
	log_print("  Discards\t\t%s", get_pool_discards_name(seg->discards));
	log_print("  Thin count\t\t%u",
		  dm_list_size(&seg->lv->segs_using_this_lv));
	log_print("  Transaction ID\t%" PRIu64, seg->transaction_id);
	log_print("  Zero new blocks\t%s",
		  (seg->zero_new_blocks == THIN_ZERO_YES) ? "yes" : "no");

	log_print(" ");
}

static int _thin_pool_add_message(struct lv_segment *seg,
				  const char *key,
				  const struct dm_config_node *sn)
{
	const char *lv_name = NULL;
	struct logical_volume *lv = NULL;
	uint32_t delete_id = 0;
	dm_thin_message_t type;

	/* Message must have only one from: create, delete */
	if (dm_config_get_str(sn, "create", &lv_name)) {
		if (!(lv = find_lv(seg->lv->vg, lv_name)))
			return SEG_LOG_ERROR("Unknown LV %s for create message in",
					     lv_name);
		/* FIXME: switch to _SNAP later, if the created LV has an origin */
		type = DM_THIN_MESSAGE_CREATE_THIN;
	} else if (dm_config_get_uint32(sn, "delete", &delete_id))
		type = DM_THIN_MESSAGE_DELETE;
	else
		return SEG_LOG_ERROR("Unknown message in");

	if (!attach_pool_message(seg, type, lv, delete_id, 1))
		return_0;

	return 1;
}

static int _thin_pool_text_import(struct lv_segment *seg,
				  const struct dm_config_node *sn,
				  struct dm_hash_table *pv_hash __attribute__((unused)))
{
	const char *lv_name;
	struct logical_volume *pool_data_lv, *pool_metadata_lv;
	const char *discards_str = NULL;
	uint32_t zero = 0;

	if (!dm_config_get_str(sn, "metadata", &lv_name))
		return SEG_LOG_ERROR("Metadata must be a string in");

	if (!(pool_metadata_lv = find_lv(seg->lv->vg, lv_name)))
		return SEG_LOG_ERROR("Unknown metadata %s in", lv_name);

	if (!dm_config_get_str(sn, "pool", &lv_name))
		return SEG_LOG_ERROR("Pool must be a string in");

	if (!(pool_data_lv = find_lv(seg->lv->vg, lv_name)))
		return SEG_LOG_ERROR("Unknown pool %s in", lv_name);

	if (!attach_pool_data_lv(seg, pool_data_lv))
		return_0;

	if (!attach_pool_metadata_lv(seg, pool_metadata_lv))
		return_0;

	if (!dm_config_get_uint64(sn, "transaction_id", &seg->transaction_id))
		return SEG_LOG_ERROR("Could not read transaction_id for");

	if (!dm_config_get_uint32(sn, "chunk_size", &seg->chunk_size))
		return SEG_LOG_ERROR("Could not read chunk_size");

	if (dm_config_has_node(sn, "discards") &&
	    !dm_config_get_str(sn, "discards", &discards_str))
		return SEG_LOG_ERROR("Could not read discards for");

	if (!discards_str)
		seg->discards = THIN_DISCARDS_IGNORE;
	else if (!set_pool_discards(&seg->discards, discards_str))
		return SEG_LOG_ERROR("Discards option unsupported for");

	if ((seg->chunk_size < DM_THIN_MIN_DATA_BLOCK_SIZE) ||
	    (seg->chunk_size > DM_THIN_MAX_DATA_BLOCK_SIZE))
		return SEG_LOG_ERROR("Unsupported value %u for chunk_size",
				     seg->device_id);

	if (dm_config_has_node(sn, "zero_new_blocks") &&
	    !dm_config_get_uint32(sn, "zero_new_blocks", &zero))
		return SEG_LOG_ERROR("Could not read zero_new_blocks for");

	seg->zero_new_blocks = (zero) ? THIN_ZERO_YES : THIN_ZERO_NO;

	/* Read messages */
	for (; sn; sn = sn->sib)
		if (!(sn->v) && !_thin_pool_add_message(seg, sn->key, sn->child))
			return_0;

	return 1;
}

static int _thin_pool_text_import_area_count(const struct dm_config_node *sn,
					     uint32_t *area_count)
{
	*area_count = 1;

	return 1;
}

static int _thin_pool_text_export(const struct lv_segment *seg, struct formatter *f)
{
	unsigned cnt = 0;
	const struct lv_thin_message *tmsg;

	outf(f, "metadata = \"%s\"", seg->metadata_lv->name);
	outf(f, "pool = \"%s\"", seg_lv(seg, 0)->name);
	outf(f, "transaction_id = %" PRIu64, seg->transaction_id);
	outsize(f, (uint64_t) seg->chunk_size,
		"chunk_size = %u", seg->chunk_size);

	switch (seg->discards) {
	case THIN_DISCARDS_PASSDOWN:
	case THIN_DISCARDS_NO_PASSDOWN:
	case THIN_DISCARDS_IGNORE:
		outf(f, "discards = \"%s\"", get_pool_discards_name(seg->discards));
		break;
	default:
		log_error(INTERNAL_ERROR "Invalid discards value %d.", seg->discards);
		return 0;
	}

	if (seg->zero_new_blocks == THIN_ZERO_YES)
		outf(f, "zero_new_blocks = 1");
	else if (seg->zero_new_blocks != THIN_ZERO_NO) {
		log_error(INTERNAL_ERROR "Invalid zero new blocks value %d.",
			  seg->zero_new_blocks);
		return 0;
	}

	dm_list_iterate_items(tmsg, &seg->thin_messages) {
		/* Extra validation */
		switch (tmsg->type) {
		case DM_THIN_MESSAGE_CREATE_SNAP:
		case DM_THIN_MESSAGE_CREATE_THIN:
			if (!lv_is_thin_volume(tmsg->u.lv)) {
				log_error(INTERNAL_ERROR
					  "LV %s is not a thin volume.",
					  tmsg->u.lv->name);
				return 0;
			}
			break;
		default:
			break;
		}

		if (!cnt)
			outnl(f);

		outf(f, "message%d {", ++cnt);
		out_inc_indent(f);

		switch (tmsg->type) {
		case DM_THIN_MESSAGE_CREATE_SNAP:
		case DM_THIN_MESSAGE_CREATE_THIN:
			outf(f, "create = \"%s\"", tmsg->u.lv->name);
			break;
		case DM_THIN_MESSAGE_DELETE:
			outf(f, "delete = %d", tmsg->u.delete_id);
			break;
		default:
			log_error(INTERNAL_ERROR "Passed unsupported message.");
			return 0;
		}

		out_dec_indent(f);
		outf(f, "}");
	}

	return 1;
}

#ifdef DEVMAPPER_SUPPORT
static int _thin_target_present(struct cmd_context *cmd,
				const struct lv_segment *seg __attribute__((unused)),
				unsigned *attributes);

static int _thin_pool_modules_needed(struct dm_pool *mem,
				     const struct lv_segment *seg __attribute__((unused)),
				     struct dm_list *modules)
{
	if (!str_list_add(mem, modules, _thin_pool_module)) {
		log_error("String list allocation failed for thin_pool.");
		return 0;
	}

	return 1;
}

static int _thin_modules_needed(struct dm_pool *mem,
				const struct lv_segment *seg,
				struct dm_list *modules)
{
	if (!_thin_pool_modules_needed(mem, seg, modules))
		return_0;

	if (!str_list_add(mem, modules, _thin_module)) {
		log_error("String list allocation failed for thin.");
		return 0;
	}

	return 1;
}

static int _thin_pool_add_target_line(struct dev_manager *dm,
				      struct dm_pool *mem,
				      struct cmd_context *cmd,
				      void **target_state __attribute__((unused)),
				      struct lv_segment *seg,
				      const struct lv_activate_opts *laopts,
				      struct dm_tree_node *node, uint64_t len,
				      uint32_t *pvmove_mirror_count __attribute__((unused)))
{
	static int _no_discards = 0;
	static int _no_error_if_no_space = 0;
	char *metadata_dlid, *pool_dlid;
	const struct lv_thin_message *lmsg;
	const struct logical_volume *origin;
	struct lvinfo info;
	uint64_t transaction_id = 0;
	unsigned attr;
	uint64_t low_water_mark;
	int threshold;

	if (!_thin_target_present(cmd, NULL, &attr))
		return_0;

	if (!seg->metadata_lv) {
		log_error(INTERNAL_ERROR "Thin pool is missing metadata device.");
		return 0;
	}

	if (!(attr & THIN_FEATURE_BLOCK_SIZE) &&
	    !is_power_of_2(seg->chunk_size)) {
		log_error("Thin pool target does not support %s chunk size (needs"
			  " kernel >= 3.6).", display_size(cmd, seg->chunk_size));
		return 0;
	}

	if (!(metadata_dlid = build_dm_uuid(mem, seg->metadata_lv, NULL))) {
		log_error("Failed to build uuid for metadata LV %s.",
			  display_lvname(seg->metadata_lv));
		return 0;
	}

	if (!(pool_dlid = build_dm_uuid(mem, seg_lv(seg, 0), NULL))) {
		log_error("Failed to build uuid for pool LV %s.",
			  display_lvname(seg_lv(seg, 0)));
		return 0;
	}

	threshold = find_config_tree_int(seg->lv->vg->cmd,
					 activation_thin_pool_autoextend_threshold_CFG,
					 lv_config_profile(seg->lv));
	if (threshold < 50)
		threshold = 50;
	if (threshold < 100)
		/* Translate to number of free pool blocks to trigger watermark */
		low_water_mark = len / seg->chunk_size * (100 - threshold) / 100;
	else
		low_water_mark = 0;

	if (!dm_tree_node_add_thin_pool_target(node, len,
					       seg->transaction_id,
					       metadata_dlid, pool_dlid,
					       seg->chunk_size, low_water_mark,
					       (seg->zero_new_blocks == THIN_ZERO_YES) ? 0 : 1))
		return_0;

	if (attr & THIN_FEATURE_DISCARDS) {
		/* Use ignore for discards ignore or non-power-of-2 chunk_size and <1.5 target */
		/* FIXME: Check whether underlying dev supports discards */
		if (((!(attr & THIN_FEATURE_DISCARDS_NON_POWER_2) &&
		      !is_power_of_2(seg->chunk_size)) ||
		     (seg->discards == THIN_DISCARDS_IGNORE))) {
			if (!dm_tree_node_set_thin_pool_discard(node, 1, 0))
				return_0;
		} else if (!dm_tree_node_set_thin_pool_discard(node, 0,
							       (seg->discards == THIN_DISCARDS_NO_PASSDOWN)))
			return_0;
	} else if (seg->discards != THIN_DISCARDS_IGNORE)
		log_warn_suppress(_no_discards++, "WARNING: Thin pool target does "
				  "not support discards (needs kernel >= 3.4).");

	if (attr & THIN_FEATURE_ERROR_IF_NO_SPACE)
		dm_tree_node_set_thin_pool_error_if_no_space(node, lv_is_error_when_full(seg->lv));
	else if (lv_is_error_when_full(seg->lv))
		log_warn_suppress(_no_error_if_no_space++, "WARNING: Thin pool target does "
				  "not support error if no space (needs version >= 1.10).");

	/*
	 * Add messages only for activation tree.
	 * Otherwise avoid checking for existence of suspended origin.
	 * Also transation_id is checked only when snapshot origin is active.
	 * (This might change later)
	 */
	if (!laopts->send_messages)
		return 1;

	dm_list_iterate_items(lmsg, &seg->thin_messages) {
		switch (lmsg->type) {
		case DM_THIN_MESSAGE_CREATE_THIN:
			origin = first_seg(lmsg->u.lv)->origin;
			/* Check if the origin is suspended */
			if (origin && lv_info(cmd, origin, 1, &info, 0, 0) &&
			    info.exists && !info.suspended) {
				/* Origin is not suspended, but the transaction may have been
				 * already transfered, so test for transaction_id and
				 * allow to pass in the message for dmtree processing
				 * so it will skip all messages later.
				 */
				if (!lv_thin_pool_transaction_id(seg->lv, &transaction_id))
					return_0; /* Thin pool should exist and work */
				if ((transaction_id + 1) != seg->transaction_id) {
					log_error("Can't create snapshot %s as origin %s is not suspended.",
						  lmsg->u.lv->name, origin->name);
					return 0;
				}
			}
			log_debug_activation("Thin pool create_%s %s.", (!origin) ? "thin" : "snap", lmsg->u.lv->name);
			if (!dm_tree_node_add_thin_pool_message(node,
								(!origin) ? lmsg->type : DM_THIN_MESSAGE_CREATE_SNAP,
								first_seg(lmsg->u.lv)->device_id,
								(!origin) ? 0 : first_seg(origin)->device_id))
				return_0;
			break;
		case DM_THIN_MESSAGE_DELETE:
			log_debug_activation("Thin pool delete %u.", lmsg->u.delete_id);
			if (!dm_tree_node_add_thin_pool_message(node,
								lmsg->type,
								lmsg->u.delete_id, 0))
				return_0;
			break;
		default:
			log_error(INTERNAL_ERROR "Unsupported message.");
			return 0;
		}
	}

	if (!dm_list_empty(&seg->thin_messages)) {
		/* Messages were passed, modify transaction_id as the last one */
		log_debug_activation("Thin pool set transaction id %" PRIu64 ".", seg->transaction_id);
		if (!dm_tree_node_add_thin_pool_message(node,
							DM_THIN_MESSAGE_SET_TRANSACTION_ID,
							seg->transaction_id - 1,
							seg->transaction_id))
			return_0;
	}

	return 1;
}

static int _thin_pool_target_percent(void **target_state __attribute__((unused)),
				     dm_percent_t *percent,
				     struct dm_pool *mem,
				     struct cmd_context *cmd __attribute__((unused)),
				     struct lv_segment *seg,
				     char *params,
				     uint64_t *total_numerator,
				     uint64_t *total_denominator)
{
	struct dm_status_thin_pool *s;

	if (!dm_get_status_thin_pool(mem, params, &s))
		return_0;

	if (s->fail || s->error)
		*percent = DM_PERCENT_INVALID;
	/* With 'seg' report metadata percent, otherwice data percent */
	else if (seg) {
		*percent = dm_make_percent(s->used_metadata_blocks,
					   s->total_metadata_blocks);
		*total_numerator += s->used_metadata_blocks;
		*total_denominator += s->total_metadata_blocks;
	} else {
		*percent = dm_make_percent(s->used_data_blocks,
					   s->total_data_blocks);
		*total_numerator += s->used_data_blocks;
		*total_denominator += s->total_data_blocks;
	}

	return 1;
}

#  ifdef DMEVENTD
/* FIXME Cache this */
static int _target_registered(struct lv_segment *seg, int *pending, int *monitored)
{
	return target_registered_with_dmeventd(seg->lv->vg->cmd,
					       seg->segtype->dso,
					       seg->lv, pending, monitored);
}

/* FIXME This gets run while suspended and performs banned operations. */
static int _target_set_events(struct lv_segment *seg, int evmask, int set)
{
	/* FIXME Make timeout (10) configurable */
	return target_register_events(seg->lv->vg->cmd,
				      seg->segtype->dso,
				      seg->lv, evmask, set, 10);
}

static int _target_register_events(struct lv_segment *seg,
				   int events)
{
	return _target_set_events(seg, events, 1);
}

static int _target_unregister_events(struct lv_segment *seg,
				     int events)
{
	return _target_set_events(seg, events, 0);
}

#  endif /* DMEVENTD */
#endif /* DEVMAPPER_SUPPORT */

static void _thin_display(const struct lv_segment *seg)
{
	log_print("  Device ID\t\t%u", seg->device_id);

	log_print(" ");
}

static int _thin_text_import(struct lv_segment *seg,
			     const struct dm_config_node *sn,
			     struct dm_hash_table *pv_hash __attribute__((unused)))
{
	const char *lv_name;
	struct logical_volume *pool_lv, *origin = NULL, *external_lv = NULL, *merge_lv = NULL;
	struct generic_logical_volume *indirect_origin = NULL;

	if (!dm_config_get_str(sn, "thin_pool", &lv_name))
		return SEG_LOG_ERROR("Thin pool must be a string in");

	if (!(pool_lv = find_lv(seg->lv->vg, lv_name)))
		return SEG_LOG_ERROR("Unknown thin pool %s in", lv_name);

	if (!dm_config_get_uint64(sn, "transaction_id", &seg->transaction_id))
		return SEG_LOG_ERROR("Could not read transaction_id for");

	if (dm_config_has_node(sn, "origin")) {
		if (!dm_config_get_str(sn, "origin", &lv_name))
			return SEG_LOG_ERROR("Origin must be a string in");

		if (!(origin = find_lv(seg->lv->vg, lv_name)))
			return SEG_LOG_ERROR("Unknown origin %s in", lv_name);
	}

	if (dm_config_has_node(sn, "merge")) {
		if (!dm_config_get_str(sn, "merge", &lv_name))
			return SEG_LOG_ERROR("Merge lv must be a string in");
		if (!(merge_lv = find_lv(seg->lv->vg, lv_name)))
			return SEG_LOG_ERROR("Unknown merge lv %s in", lv_name);
	}

	if (!dm_config_get_uint32(sn, "device_id", &seg->device_id))
		return SEG_LOG_ERROR("Could not read device_id for");

	if (seg->device_id > DM_THIN_MAX_DEVICE_ID)
		return SEG_LOG_ERROR("Unsupported value %u for device_id",
				     seg->device_id);

	if (dm_config_has_node(sn, "external_origin")) {
		if (!dm_config_get_str(sn, "external_origin", &lv_name))
			return SEG_LOG_ERROR("External origin must be a string in");

		if (!(external_lv = find_lv(seg->lv->vg, lv_name)))
			return SEG_LOG_ERROR("Unknown external origin %s in", lv_name);
	}

	if (!attach_pool_lv(seg, pool_lv, origin, indirect_origin, merge_lv))
		return_0;

	if (!attach_thin_external_origin(seg, external_lv))
		return_0;

	return 1;
}

static int _thin_text_export(const struct lv_segment *seg, struct formatter *f)
{
	outf(f, "thin_pool = \"%s\"", seg->pool_lv->name);
	outf(f, "transaction_id = %" PRIu64, seg->transaction_id);
	outf(f, "device_id = %d", seg->device_id);

	if (seg->external_lv)
		outf(f, "external_origin = \"%s\"", seg->external_lv->name);
	if (seg->origin)
		outf(f, "origin = \"%s\"", seg->origin->name);

	if (seg->merge_lv)
		outf(f, "merge = \"%s\"", seg->merge_lv->name);

	return 1;
}

#ifdef DEVMAPPER_SUPPORT
static int _thin_add_target_line(struct dev_manager *dm,
				 struct dm_pool *mem,
				 struct cmd_context *cmd __attribute__((unused)),
				 void **target_state __attribute__((unused)),
				 struct lv_segment *seg,
				 const struct lv_activate_opts *laopts,
				 struct dm_tree_node *node, uint64_t len,
				 uint32_t *pvmove_mirror_count __attribute__((unused)))
{
	char *pool_dlid, *external_dlid;
	uint32_t device_id = seg->device_id;
	unsigned attr;

	if (!seg->pool_lv) {
		log_error(INTERNAL_ERROR "Segment %s has no pool.",
			  seg->lv->name);
		return 0;
	}
	if (!(pool_dlid = build_dm_uuid(mem, seg->pool_lv, lv_layer(seg->pool_lv)))) {
		log_error("Failed to build uuid for pool LV %s.",
			  seg->pool_lv->name);
		return 0;
	}

	if (!laopts->no_merging) {
		if (seg->merge_lv) {
			log_error(INTERNAL_ERROR "Failed to add merged segment of %s.",
				  seg->lv->name);
			return 0;
		}
		/*
		 * merge support for thinp snapshots is implemented by
		 * simply swapping the thinp device_id of the snapshot
		 * and origin.
		 */
		if (lv_is_merging_origin(seg->lv) && seg_is_thin_volume(find_snapshot(seg->lv)))
			/* origin, use merging snapshot's device_id */
			device_id = find_snapshot(seg->lv)->device_id;
	}

	if (!dm_tree_node_add_thin_target(node, len, pool_dlid, device_id))
		return_0;

	/* Add external origin LV */
	if (seg->external_lv) {
		if (!pool_supports_external_origin(first_seg(seg->pool_lv), seg->external_lv))
			return_0;
		if (seg->external_lv->size < seg->lv->size) {
			/* Validate target supports smaller external origin */
			if (!_thin_target_present(cmd, NULL, &attr) ||
			    !(attr & THIN_FEATURE_EXTERNAL_ORIGIN_EXTEND)) {
				log_error("Thin target does not support smaller size of external origin LV %s.",
					  seg->external_lv->name);
				return 0;
			}
		}
		if (!(external_dlid = build_dm_uuid(mem, seg->external_lv,
						    lv_layer(seg->external_lv)))) {
			log_error("Failed to build uuid for external origin LV %s.",
				  seg->external_lv->name);
			return 0;
		}
		if (!dm_tree_node_set_thin_external_origin(node, external_dlid))
			return_0;
	}

	return 1;
}

static int _thin_target_percent(void **target_state __attribute__((unused)),
				dm_percent_t *percent,
				struct dm_pool *mem,
				struct cmd_context *cmd __attribute__((unused)),
				struct lv_segment *seg,
				char *params,
				uint64_t *total_numerator,
				uint64_t *total_denominator)
{
	struct dm_status_thin *s;
	uint64_t csize;

	/* Status for thin device is in sectors */
	if (!dm_get_status_thin(mem, params, &s))
		return_0;

	if (s->fail)
		*percent = DM_PERCENT_INVALID;
	else if (seg) {
		/* Pool allocates whole chunk so round-up to nearest one */
		csize = first_seg(seg->pool_lv)->chunk_size;
		csize = ((seg->lv->size + csize - 1) / csize) * csize;
		if (s->mapped_sectors > csize) {
			log_warn("WARNING: LV %s maps %s while the size is only %s.",
				 display_lvname(seg->lv),
				 display_size(cmd, s->mapped_sectors),
				 display_size(cmd, csize));
			/* Don't show nonsense numbers like i.e. 1000% full */
			s->mapped_sectors = csize;
		}

		*percent = dm_make_percent(s->mapped_sectors, csize);
		*total_denominator += csize;
	} else {
		/* No lv_segment info here */
		*percent = DM_PERCENT_INVALID;
		/* FIXME: Using denominator to pass the mapped info upward? */
		*total_denominator += s->highest_mapped_sector;
	}

	*total_numerator += s->mapped_sectors;

	return 1;
}

static int _thin_target_present(struct cmd_context *cmd,
				const struct lv_segment *seg __attribute__((unused)),
				unsigned *attributes)
{
	/* List of features with their kernel target version */
	static const struct feature {
		uint32_t maj;
		uint32_t min;
		unsigned thin_feature;
		const char *feature;
	} _features[] = {
		{ 1, 1, THIN_FEATURE_DISCARDS, "discards" },
		{ 1, 1, THIN_FEATURE_EXTERNAL_ORIGIN, "external_origin" },
		{ 1, 4, THIN_FEATURE_BLOCK_SIZE, "block_size" },
		{ 1, 5, THIN_FEATURE_DISCARDS_NON_POWER_2, "discards_non_power_2" },
		{ 1, 10, THIN_FEATURE_METADATA_RESIZE, "metadata_resize" },
		{ 1, 10, THIN_FEATURE_ERROR_IF_NO_SPACE, "error_if_no_space" },
		{ 1, 13, THIN_FEATURE_EXTERNAL_ORIGIN_EXTEND, "external_origin_extend" },
	};

	static const char _lvmconf[] = "global/thin_disabled_features";
	static int _checked = 0;
	static int _present = 0;
	static unsigned _attrs = 0;
	uint32_t maj, min, patchlevel;
	unsigned i;
	const struct dm_config_node *cn;
	const struct dm_config_value *cv;
	const char *str;

	if (!activation())
		return 0;

	if (!_checked) {
		_checked = 1;

		if (!(_present = target_present(cmd, _thin_pool_module, 1)))
			return 0;

		if (!target_version(_thin_pool_module, &maj, &min, &patchlevel))
			return_0;

		for (i = 0; i < DM_ARRAY_SIZE(_features); ++i)
			if ((maj > _features[i].maj) ||
			    (maj == _features[i].maj && min >= _features[i].min))
				_attrs |= _features[i].thin_feature;
			else
				log_very_verbose("Target %s does not support %s.",
						 _thin_pool_module,
						 _features[i].feature);
	}

	if (attributes) {
		if (!_feature_mask) {
			/* Support runtime lvm.conf changes, N.B. avoid 32 feature */
			if ((cn = find_config_tree_array(cmd, global_thin_disabled_features_CFG, NULL))) {
				for (cv = cn->v; cv; cv = cv->next) {
					if (cv->type != DM_CFG_STRING) {
						log_warn("WARNING: Ignoring invalid string in config file %s.",
							  _lvmconf);
						continue;
					}
					str = cv->v.str;
					if (!*str)
						continue;
					for (i = 0; i < DM_ARRAY_SIZE(_features); ++i)
						if (strcasecmp(str, _features[i].feature) == 0)
							_feature_mask |= _features[i].thin_feature;
				}
			}
			_feature_mask = ~_feature_mask;
			for (i = 0; i < DM_ARRAY_SIZE(_features); ++i)
				if ((_attrs & _features[i].thin_feature) &&
				    !(_feature_mask & _features[i].thin_feature))
					log_very_verbose("Target %s %s support disabled by %s",
							 _thin_pool_module,
							 _features[i].feature, _lvmconf);
		}
		*attributes = _attrs & _feature_mask;
	}

	return _present;
}
#endif

static void _thin_destroy(struct segment_type *segtype)
{
	free((void *) segtype->dso);
	free(segtype);
}

static struct segtype_handler _thin_pool_ops = {
	.display = _thin_pool_display,
	.text_import = _thin_pool_text_import,
	.text_import_area_count = _thin_pool_text_import_area_count,
	.text_export = _thin_pool_text_export,
#ifdef DEVMAPPER_SUPPORT
	.add_target_line = _thin_pool_add_target_line,
	.target_percent = _thin_pool_target_percent,
	.target_present = _thin_target_present,
	.modules_needed = _thin_pool_modules_needed,
#  ifdef DMEVENTD
	.target_monitored = _target_registered,
	.target_monitor_events = _target_register_events,
	.target_unmonitor_events = _target_unregister_events,
#  endif /* DMEVENTD */
#endif
	.destroy = _thin_destroy,
};

static struct segtype_handler _thin_ops = {
	.display = _thin_display,
	.text_import = _thin_text_import,
	.text_export = _thin_text_export,
#ifdef DEVMAPPER_SUPPORT
	.add_target_line = _thin_add_target_line,
	.target_percent = _thin_target_percent,
	.target_present = _thin_target_present,
	.modules_needed = _thin_modules_needed,
#endif
	.destroy = _thin_destroy,
};

#ifdef THIN_INTERNAL
int init_thin_segtypes(struct cmd_context *cmd, struct segtype_library *seglib)
#else /* Shared */
int init_multiple_segtypes(struct cmd_context *cmd, struct segtype_library *seglib);
int init_multiple_segtypes(struct cmd_context *cmd, struct segtype_library *seglib)
#endif
{
	static const struct {
		struct segtype_handler *ops;
		const char name[16];
		uint32_t flags;
	} reg_segtypes[] = {
		{ &_thin_pool_ops, "thin-pool", SEG_THIN_POOL | SEG_CANNOT_BE_ZEROED |
		SEG_ONLY_EXCLUSIVE | SEG_CAN_ERROR_WHEN_FULL },
		/* FIXME Maybe use SEG_THIN_VOLUME instead of SEG_VIRTUAL */
		{ &_thin_ops, "thin", SEG_THIN_VOLUME | SEG_VIRTUAL | SEG_ONLY_EXCLUSIVE }
	};

	struct segment_type *segtype;
	unsigned i;

	for (i = 0; i < DM_ARRAY_SIZE(reg_segtypes); ++i) {
		segtype = zalloc(sizeof(*segtype));

		if (!segtype) {
			log_error("Failed to allocate memory for %s segtype",
				  reg_segtypes[i].name);
			return 0;
		}

		segtype->ops = reg_segtypes[i].ops;
		segtype->name = reg_segtypes[i].name;
		segtype->flags = reg_segtypes[i].flags;

#ifdef DEVMAPPER_SUPPORT
#  ifdef DMEVENTD
		segtype->dso = get_monitor_dso_path(cmd, dmeventd_thin_library_CFG);

		if ((reg_segtypes[i].flags & SEG_THIN_POOL) &&
		    segtype->dso)
			segtype->flags |= SEG_MONITORED;
#  endif /* DMEVENTD */
#endif
		if (!lvm_register_segtype(seglib, segtype))
			/* segtype is already destroyed */
			return_0;

		log_very_verbose("Initialised segtype: %s", segtype->name);
	}


	/* Reset mask for recalc */
	_feature_mask = 0;

	return 1;
}
