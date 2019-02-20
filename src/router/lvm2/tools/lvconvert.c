/*
 * Copyright (C) 2005-2016 Red Hat, Inc. All rights reserved.
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

#include "tools.h"

#include "lib/lvmpolld/polldaemon.h"
#include "lib/metadata/lv_alloc.h"
#include "lvconvert_poll.h"

#define MAX_PDATA_ARGS	10	/* Max number of accepted args for d-m-p-d tools */

typedef enum {
	/* Split:
	 *   For a mirrored or raid LV, split mirror into two mirrors, optionally tracking
	 *     future changes to the main mirror to allow future recombination.
	 */
	CONV_SPLIT_MIRRORS = 2,

	/* Every other segment type or mirror log conversion we haven't separated out */
	CONV_OTHER = 3,
} conversion_type_t;

struct lvconvert_params {
	/* Exactly one of these 12 command categories is determined */
	int keep_mimages;	/* 2 */	/* --splitmirrors */
	/* other */		/* 3 */

	/* FIXME Eliminate all cases where more than one of the above are set then use conv_type instead */
	conversion_type_t	conv_type;

	int track_changes;	/* CONV_SPLIT_MIRRORS is set */

	int corelog;		/* Equivalent to --mirrorlog core */
	int mirrorlog;		/* Only one of corelog and mirrorlog may be set */

	int mirrors_supplied;	/* When type_str is not set, this may be set with keep_mimages for --splitmirrors */
	const char *type_str;	/* When this is set, mirrors_supplied may optionally also be set */
				/* Holds what you asked for based on --type or other arguments, else "" */

	const struct segment_type *segtype;	/* Holds what segment type you will get */

	int force;
	int yes;
	int zero;

	const char *lv_name;
	const char *lv_split_name;
	const char *lv_name_full;
	const char *vg_name;
	int wait_completion;
	int need_polling;

	uint32_t region_size;
	unsigned region_size_supplied;

	uint32_t mirrors;
	sign_t mirrors_sign;
	uint32_t stripes;
	uint32_t stripe_size;
	unsigned stripes_supplied;
	unsigned stripe_size_supplied;
	uint32_t read_ahead;

	unsigned target_attr;

	alloc_policy_t alloc;

	int pv_count;
	char **pvs;
	struct dm_list *pvh;

	struct logical_volume *lv_to_poll;
	struct dm_list idls;

	const char *origin_name;
};

struct convert_poll_id_list {
	struct dm_list list;
	struct poll_operation_id *id;
	unsigned is_merging_origin:1;
	unsigned is_merging_origin_thin:1;
};

/* FIXME Temporary function until the enum replaces the separate variables */
static void _set_conv_type(struct lvconvert_params *lp, int conv_type)
{
	if (lp->conv_type != CONV_OTHER)
		log_error(INTERNAL_ERROR "Changing conv_type from %d to %d.", lp->conv_type, conv_type);

	lp->conv_type = conv_type;
}

static int _raid0_type_requested(const char *type_str)
{
	return (!strcmp(type_str, SEG_TYPE_NAME_RAID0) || !strcmp(type_str, SEG_TYPE_NAME_RAID0_META));
}

/* mirror/raid* (1,10,4,5,6 and their variants) reshape */
static int _mirror_or_raid_type_requested(struct cmd_context *cmd, const char *type_str)
{
	return (arg_is_set(cmd, mirrors_ARG) || !strcmp(type_str, SEG_TYPE_NAME_MIRROR) ||
		(!strncmp(type_str, SEG_TYPE_NAME_RAID, 4) && !_raid0_type_requested(type_str)));
}

static int _linear_type_requested(const char *type_str)
{
	return (!strcmp(type_str, SEG_TYPE_NAME_LINEAR));
}

static int _striped_type_requested(const char *type_str)
{
	return (!strcmp(type_str, SEG_TYPE_NAME_STRIPED) || _linear_type_requested(type_str));
}

static int _read_conversion_type(struct cmd_context *cmd,
				 struct lvconvert_params *lp)
{

	const char *type_str = arg_str_value(cmd, type_ARG, "");

	lp->type_str =  type_str;
	if (!lp->type_str[0])
		return 1;

	/* FIXME: Check thin-pool and thin more thoroughly! */
	if (!strcmp(type_str, SEG_TYPE_NAME_SNAPSHOT) || _striped_type_requested(type_str) ||
	    !strncmp(type_str, SEG_TYPE_NAME_RAID, 4) || !strcmp(type_str, SEG_TYPE_NAME_MIRROR) ||
	    !strcmp(type_str, SEG_TYPE_NAME_CACHE_POOL) || !strcmp(type_str, SEG_TYPE_NAME_CACHE) ||
	    !strcmp(type_str, SEG_TYPE_NAME_THIN_POOL) || !strcmp(type_str, SEG_TYPE_NAME_THIN))
		return 1;

	log_error("Conversion using --type %s is not supported.", type_str);

	return 0;
}

static int _read_params(struct cmd_context *cmd, struct lvconvert_params *lp)
{
	const char *vg_name = NULL;

	if (!_read_conversion_type(cmd, lp))
		return_0;

	if (!arg_is_set(cmd, background_ARG))
		lp->wait_completion = 1;

	if (arg_is_set(cmd, corelog_ARG))
		lp->corelog = 1;

	if (arg_is_set(cmd, mirrorlog_ARG)) {
		if (lp->corelog) {
			log_error("--mirrorlog and --corelog are incompatible.");
			return 0;
		}
		lp->mirrorlog = 1;
	}

	if (arg_is_set(cmd, trackchanges_ARG))
		lp->track_changes = 1;

	/*
	 * The '--splitmirrors n' argument is equivalent to '--mirrors -n'
	 * (note the minus sign), except that it signifies the additional
	 * intent to keep the mimage that is detached, rather than
	 * discarding it.
	 */
	if (arg_is_set(cmd, splitmirrors_ARG)) {
		if ((lp->lv_split_name = arg_str_value(cmd, name_ARG, NULL))) {
			if (!validate_restricted_lvname_param(cmd, &vg_name, &lp->lv_split_name))
				return_0;
		}

		if (_mirror_or_raid_type_requested(cmd, lp->type_str)) {
			log_error("--mirrors/--type mirror/--type raid* and --splitmirrors are "
				  "mutually exclusive.");
			return 0;
		}

		if (!arg_is_set(cmd, name_ARG) && !lp->track_changes) {
			log_error("Please name the new logical volume using '--name'");
			return 0;
		}

		if ((lp->lv_split_name = arg_str_value(cmd, name_ARG, NULL))) {
			if (!validate_restricted_lvname_param(cmd, &vg_name, &lp->lv_split_name))
				return_0;
		}

		lp->keep_mimages = 1;
		_set_conv_type(lp, CONV_SPLIT_MIRRORS);
		lp->mirrors = arg_uint_value(cmd, splitmirrors_ARG, 0);
		lp->mirrors_sign = SIGN_MINUS;
	}

	/* If no other case was identified, then use of --stripes means --type striped */
	if (!arg_is_set(cmd, type_ARG) && !*lp->type_str &&
	    !lp->mirrorlog && !lp->corelog &&
	    (arg_is_set(cmd, stripes_long_ARG) || arg_is_set(cmd, stripesize_ARG)))
		lp->type_str = SEG_TYPE_NAME_STRIPED;

	if ((arg_is_set(cmd, stripes_long_ARG) || arg_is_set(cmd, stripesize_ARG)) &&
	    !(_mirror_or_raid_type_requested(cmd, lp->type_str) || _striped_type_requested(lp->type_str) ||
	      _raid0_type_requested(lp->type_str) || arg_is_set(cmd, thinpool_ARG))) {
		log_error("--stripes or --stripesize argument is only valid "
			  "with --mirrors/--type mirror/--type raid*/--type striped/--type linear, --repair and --thinpool");
		return 0;
	}

	if (arg_is_set(cmd, mirrors_ARG)) {
		/* --splitmirrors is the mechanism for detaching and keeping a mimage */
		lp->mirrors_supplied = 1;
		lp->mirrors = arg_uint_value(cmd, mirrors_ARG, 0);
		lp->mirrors_sign = arg_sign_value(cmd, mirrors_ARG, SIGN_NONE);
	}

	lp->alloc = (alloc_policy_t) arg_uint_value(cmd, alloc_ARG, ALLOC_INHERIT);

	/*
	 * Final checking of each case:
	 *   lp->keep_mimages
	 *   --type mirror|raid  lp->mirrorlog lp->corelog
	 *   --type raid0|striped
	 */
	switch(lp->conv_type) {
	case CONV_SPLIT_MIRRORS:
                break;

	case CONV_OTHER:
		if (arg_is_set(cmd, regionsize_ARG)) {
			lp->region_size = arg_uint_value(cmd, regionsize_ARG, 0);
			lp->region_size_supplied = 1;
		} else {
			lp->region_size = get_default_region_size(cmd);
			lp->region_size_supplied = 0;
		}

		if (_mirror_or_raid_type_requested(cmd, lp->type_str) ||
			   lp->mirrorlog || lp->corelog) { /* Mirrors (and some RAID functions) */
			if (arg_is_set(cmd, chunksize_ARG)) {
				log_error("--chunksize is only available with snapshots or pools.");
				return 0;
			}

			if (arg_is_set(cmd, zero_ARG)) {
				log_error("--zero is only available with snapshots or thin pools.");
				return 0;
			}

			/* FIXME man page says in one place that --type and --mirrors can't be mixed */
			if (lp->mirrors_supplied && !lp->mirrors)
				/* down-converting to linear/stripe? */
				lp->type_str = SEG_TYPE_NAME_STRIPED;

		} else if (_raid0_type_requested(lp->type_str) || _striped_type_requested(lp->type_str)) { /* striped or linear or raid0 */
			if (arg_from_list_is_set(cmd, "cannot be used with --type raid0 or --type striped or --type linear",
						 chunksize_ARG, corelog_ARG, mirrors_ARG, mirrorlog_ARG, zero_ARG,
						 -1))
				return_0;
		} /* else segtype will default to current type */
	}

	lp->force = arg_count(cmd, force_ARG);
	lp->yes = arg_count(cmd, yes_ARG);

	return 1;
}


static struct poll_functions _lvconvert_mirror_fns = {
	.poll_progress = poll_mirror_progress,
	.finish_copy = lvconvert_mirror_finish,
};

static struct poll_functions _lvconvert_merge_fns = {
	.poll_progress = poll_merge_progress,
	.finish_copy = lvconvert_merge_finish,
};

static struct poll_functions _lvconvert_thin_merge_fns = {
	.poll_progress = poll_thin_merge_progress,
	.finish_copy = lvconvert_merge_finish,
};

static struct poll_operation_id *_create_id(struct cmd_context *cmd,
					    const char *vg_name,
					    const char *lv_name,
					    const char *uuid)
{
	struct poll_operation_id *id;
	char lv_full_name[NAME_LEN];

	if (!vg_name || !lv_name || !uuid) {
		log_error(INTERNAL_ERROR "Wrong params for lvconvert _create_id.");
		return NULL;
	}

	if (dm_snprintf(lv_full_name, sizeof(lv_full_name), "%s/%s", vg_name, lv_name) < 0) {
		log_error(INTERNAL_ERROR "Name \"%s/%s\" is too long.", vg_name, lv_name);
		return NULL;
	}

	if (!(id = dm_pool_alloc(cmd->mem, sizeof(*id)))) {
		log_error("Poll operation ID allocation failed.");
		return NULL;
	}

	if (!(id->display_name = dm_pool_strdup(cmd->mem, lv_full_name)) ||
	    !(id->lv_name = strchr(id->display_name, '/')) ||
	    !(id->vg_name = dm_pool_strdup(cmd->mem, vg_name)) ||
	    !(id->uuid = dm_pool_strdup(cmd->mem, uuid))) {
		log_error("Failed to copy one or more poll operation ID members.");
		dm_pool_free(cmd->mem, id);
		return NULL;
	}

	id->lv_name++; /* skip over '/' */

	return id;
}

static int _lvconvert_poll_by_id(struct cmd_context *cmd, struct poll_operation_id *id,
				 unsigned background,
				 int is_merging_origin,
				 int is_merging_origin_thin)
{
	if (test_mode())
		return ECMD_PROCESSED;

	if (is_merging_origin)
		return poll_daemon(cmd, background,
				(MERGING | (is_merging_origin_thin ? THIN_VOLUME : SNAPSHOT)),
				is_merging_origin_thin ? &_lvconvert_thin_merge_fns : &_lvconvert_merge_fns,
				"Merged", id);

	return poll_daemon(cmd, background, CONVERTING,
			   &_lvconvert_mirror_fns, "Converted", id);
}

int lvconvert_poll(struct cmd_context *cmd, struct logical_volume *lv,
		   unsigned background)
{
	int r;
	struct poll_operation_id *id = _create_id(cmd, lv->vg->name, lv->name, lv->lvid.s);
	int is_merging_origin = 0;
	int is_merging_origin_thin = 0;

	if (!id) {
		log_error("Failed to allocate poll identifier for lvconvert.");
		return ECMD_FAILED;
	}

	/* FIXME: check this in polling instead */
	if (lv_is_merging_origin(lv)) {
		is_merging_origin = 1;
		is_merging_origin_thin = seg_is_thin_volume(find_snapshot(lv));
	}

	r = _lvconvert_poll_by_id(cmd, id, background, is_merging_origin, is_merging_origin_thin);

	return r;
}

static int _insert_lvconvert_layer(struct cmd_context *cmd,
				   struct logical_volume *lv)
{
	char format[NAME_LEN], layer_name[NAME_LEN];
	int i;

	/*
	 * We would like to give the same number for this layer
	 * and the newly added mimage.
	 * However, LV name of newly added mimage is determined *after*
	 * the LV name of this layer is determined.
	 *
	 * So, use generate_lv_name() to generate mimage name first
	 * and take the number from it.
	 */

	if (dm_snprintf(format, sizeof(format), "%s_mimage_%%d", lv->name) < 0) {
		log_error("lvconvert: layer name creation failed.");
		return 0;
	}

	if (!generate_lv_name(lv->vg, format, layer_name, sizeof(layer_name)) ||
	    sscanf(layer_name, format, &i) != 1) {
		log_error("lvconvert: layer name generation failed.");
		return 0;
	}

	if (dm_snprintf(layer_name, sizeof(layer_name), MIRROR_SYNC_LAYER "_%d", i) < 0) {
		log_error("layer name creation failed.");
		return 0;
	}

	if (!insert_layer_for_lv(cmd, lv, 0, layer_name)) {
		log_error("Failed to insert resync layer");
		return 0;
	}

	return 1;
}

static int _failed_mirrors_count(struct logical_volume *lv)
{
	struct lv_segment *lvseg;
	int ret = 0;
	unsigned s;

	dm_list_iterate_items(lvseg, &lv->segments) {
		if (!seg_is_mirrored(lvseg))
			return -1;
		for (s = 0; s < lvseg->area_count; s++) {
			if (seg_type(lvseg, s) == AREA_LV) {
				if (is_temporary_mirror_layer(seg_lv(lvseg, s)))
					ret += _failed_mirrors_count(seg_lv(lvseg, s));
				else if (lv_is_partial(seg_lv(lvseg, s)))
					++ ret;
			}
			else if (seg_type(lvseg, s) == AREA_PV &&
				 is_missing_pv(seg_pv(lvseg, s)))
				++ret;
		}
	}

	return ret;
}

static int _failed_logs_count(struct logical_volume *lv)
{
	int ret = 0;
	unsigned s;
	struct logical_volume *log_lv = first_seg(lv)->log_lv;
	if (log_lv && lv_is_partial(log_lv)) {
		if (lv_is_mirrored(log_lv))
			ret += _failed_mirrors_count(log_lv);
		else
			ret += 1;
	}
	for (s = 0; s < first_seg(lv)->area_count; s++) {
		if (seg_type(first_seg(lv), s) == AREA_LV &&
		    is_temporary_mirror_layer(seg_lv(first_seg(lv), s)))
			ret += _failed_logs_count(seg_lv(first_seg(lv), s));
	}
	return ret;
}


static struct dm_list *_failed_pv_list(struct volume_group *vg)
{
	struct dm_list *failed_pvs;
	struct pv_list *pvl, *new_pvl;

	if (!(failed_pvs = dm_pool_alloc(vg->vgmem, sizeof(*failed_pvs)))) {
		log_error("Allocation of list of failed_pvs failed.");
		return NULL;
	}

	dm_list_init(failed_pvs);

	dm_list_iterate_items(pvl, &vg->pvs) {
		if (!is_missing_pv(pvl->pv))
			continue;

		/*
		 * Finally, --repair will remove empty PVs.
		 * But we only want remove these which are output of repair,
		 * Do not count these which are already empty here.
		 * FIXME: code should traverse PV in LV not in whole VG.
		 * FIXME: layer violation? should it depend on vgreduce --removemising?
		 */
		if (pvl->pv->pe_alloc_count == 0)
			continue;

		if (!(new_pvl = dm_pool_zalloc(vg->vgmem, sizeof(*new_pvl)))) {
			log_error("Allocation of failed_pvs list entry failed.");
			return NULL;
		}
		new_pvl->pv = pvl->pv;
		dm_list_add(failed_pvs, &new_pvl->list);
	}

	return failed_pvs;
}

static int _is_partial_lv(struct logical_volume *lv,
			  void *baton __attribute__((unused)))
{
	return lv_is_partial(lv);
}

/*
 * Walk down the stacked mirror LV to the original mirror LV.
 */
static struct logical_volume *_original_lv(struct logical_volume *lv)
{
	struct logical_volume *next_lv = lv, *tmp_lv;

	while ((tmp_lv = find_temporary_mirror(next_lv)))
		next_lv = tmp_lv;

	return next_lv;
}

static void _lvconvert_mirrors_repair_ask(struct cmd_context *cmd,
					  int failed_log, int failed_mirrors,
					  int *replace_log, int *replace_mirrors)
{
	const char *leg_policy, *log_policy;
	int force = arg_count(cmd, force_ARG);
	int yes = arg_count(cmd, yes_ARG);

	if (arg_is_set(cmd, usepolicies_ARG)) {
		leg_policy = find_config_tree_str(cmd, activation_mirror_image_fault_policy_CFG, NULL);
		log_policy = find_config_tree_str(cmd, activation_mirror_log_fault_policy_CFG, NULL);
		*replace_mirrors = strcmp(leg_policy, "remove");
		*replace_log = strcmp(log_policy, "remove");
		return;
	}

	if (force != PROMPT) {
		*replace_log = *replace_mirrors = 0;
		return;
	}

	*replace_log = *replace_mirrors = 1;

	if (yes)
		return;

	if (failed_log &&
	    yes_no_prompt("Attempt to replace failed mirror log? [y/n]: ") == 'n')
		*replace_log = 0;

	if (failed_mirrors &&
	    yes_no_prompt("Attempt to replace failed mirror images "
			  "(requires full device resync)? [y/n]: ") == 'n')
		*replace_mirrors = 0;
}

/*
 * _get_log_count
 * @lv: the mirror LV
 *
 * Get the number of on-disk copies of the log.
 *  0  = 'core'
 *  1  = 'disk'
 *  2+ = 'mirrored'
 */
static uint32_t _get_log_count(struct logical_volume *lv)
{
	struct logical_volume *log_lv;

	log_lv = first_seg(_original_lv(lv))->log_lv;
	if (log_lv)
		return lv_mirror_count(log_lv);

	return 0;
}

static int _lv_update_mirrored_log(struct logical_volume *lv,
				   struct dm_list *operable_pvs,
				   int log_count)
{
	int old_log_count;
	struct logical_volume *log_lv;

	/*
	 * When log_count is 0, mirrored log doesn't need to be
	 * updated here but it will be removed later.
	 */
	if (!log_count)
		return 1;

	log_lv = first_seg(_original_lv(lv))->log_lv;
	if (!log_lv || !lv_is_mirrored(log_lv))
		return 1;

	old_log_count = _get_log_count(lv);
	if (old_log_count == log_count)
		return 1;

	/* Reducing redundancy of the log */
	return remove_mirror_images(log_lv, log_count,
				    is_mirror_image_removable,
				    operable_pvs, 0U);
}

static int _lv_update_log_type(struct cmd_context *cmd,
			       struct lvconvert_params *lp,
			       struct logical_volume *lv,
			       struct dm_list *operable_pvs,
			       int log_count)
{
	int old_log_count;
	uint32_t region_size = (lp) ? lp->region_size :
		first_seg(lv)->region_size;
	alloc_policy_t alloc = (lp) ? lp->alloc : lv->alloc;
	struct logical_volume *original_lv;
	struct logical_volume *log_lv;

	old_log_count = _get_log_count(lv);
	if (old_log_count == log_count)
		return 1;

	original_lv = _original_lv(lv);
	/* Remove an existing log completely */
	if (!log_count) {
		if (!remove_mirror_log(cmd, original_lv, operable_pvs,
				       arg_count(cmd, yes_ARG) ||
				       arg_count(cmd, force_ARG)))
			return_0;
		return 1;
	}

	log_lv = first_seg(original_lv)->log_lv;

	/* Adding redundancy to the log */
	if (old_log_count < log_count) {
		if (!(region_size = adjusted_mirror_region_size(cmd, lv->vg->extent_size,
								lv->le_count,
								region_size, 0,
								vg_is_clustered(lv->vg))))
			return_0;

		if (!add_mirror_log(cmd, original_lv, log_count,
				    region_size, operable_pvs, alloc))
			return_0;
		/*
		 * FIXME: This simple approach won't work in cluster mirrors,
		 *	  but it doesn't matter because we don't support
		 *	  mirrored logs in cluster mirrors.
		 */
		if (old_log_count &&
		    !lv_update_and_reload(log_lv))
			return_0;

		return 1;
	}

	/* Reducing redundancy of the log */
	return remove_mirror_images(log_lv, log_count,
				    is_mirror_image_removable, operable_pvs, 1U);
}

/*
 * Reomove missing and empty PVs from VG, if are also in provided list
 */
static void _remove_missing_empty_pv(struct volume_group *vg, struct dm_list *remove_pvs)
{
	struct pv_list *pvl, *pvl_vg, *pvlt;
	int removed = 0;

	if (!remove_pvs)
		return;

	dm_list_iterate_items(pvl, remove_pvs) {
		dm_list_iterate_items_safe(pvl_vg, pvlt, &vg->pvs) {
			if (!id_equal(&pvl->pv->id, &pvl_vg->pv->id) ||
			    !is_missing_pv(pvl_vg->pv) ||
			    pvl_vg->pv->pe_alloc_count != 0)
				continue;

			/* FIXME: duplication of vgreduce code, move this to library */
			vg->free_count -= pvl_vg->pv->pe_count;
			vg->extent_count -= pvl_vg->pv->pe_count;
			del_pvl_from_vgs(vg, pvl_vg);
			free_pv_fid(pvl_vg->pv);

			removed++;
		}
	}

	if (removed) {
		if (!vg_write(vg) || !vg_commit(vg)) {
			stack;
			return;
		}
		log_warn("WARNING: %d missing and now unallocated Physical Volumes removed from VG.", removed);
	}
}

/*
 * _lvconvert_mirrors_parse_params
 *
 * This function performs the following:
 *  1) Gets the old values of mimage and log counts
 *  2) Parses the CLI args to find the new desired values
 *  3) Adjusts 'lp->mirrors' to the appropriate absolute value.
 *     (Remember, 'lp->mirrors' is specified in terms of the number of "copies"
 *     vs. the number of mimages.  It can also be a relative value.)
 *  4) Sets 'lp->need_polling' if collapsing
 *  5) Validates other mirror params
 *
 * Returns: 1 on success, 0 on error
 */
static int _lvconvert_mirrors_parse_params(struct cmd_context *cmd,
					   struct logical_volume *lv,
					   struct lvconvert_params *lp,
					   uint32_t *old_mimage_count,
					   uint32_t *old_log_count,
					   uint32_t *new_mimage_count,
					   uint32_t *new_log_count)
{
	*old_mimage_count = lv_mirror_count(lv);
	*old_log_count = _get_log_count(lv);

	if (lv->vg->lock_type && !strcmp(lv->vg->lock_type, "sanlock") && lp->keep_mimages) {
		/* FIXME: we need to create a sanlock lock on disk for the new LV. */
		log_error("Unable to split mirrors in VG with lock_type %s", lv->vg->lock_type);
		return 0;
	}

	/*
	 * Adjusting mimage count?
	 */
	if (!lp->mirrors_supplied && !lp->keep_mimages)
		lp->mirrors = *old_mimage_count;
	else if (lp->mirrors_sign == SIGN_PLUS)
		lp->mirrors = *old_mimage_count + lp->mirrors;
	else if (lp->mirrors_sign == SIGN_MINUS)
		lp->mirrors = (*old_mimage_count > lp->mirrors) ?
			*old_mimage_count - lp->mirrors: 0;
	else
		lp->mirrors += 1;

	*new_mimage_count = lp->mirrors;

	/* Too many mimages? */
	if (lp->mirrors > DEFAULT_MIRROR_MAX_IMAGES) {
		log_error("Only up to %d images in mirror supported currently.",
			  DEFAULT_MIRROR_MAX_IMAGES);
		return 0;
	}

	/* Did the user try to subtract more legs than available? */
	if (lp->mirrors < 1) {
		log_error("Unable to reduce images by specified amount - only %d in %s",
			  *old_mimage_count, lv->name);
		return 0;
	}

	/*
	 * FIXME: It would be nice to say what we are adjusting to, but
	 * I really don't know whether to specify the # of copies or mimages.
	 */
	if (*old_mimage_count != *new_mimage_count)
		log_verbose("Adjusting mirror image count of %s", lv->name);

	/* If region size is not given by user - use value from mirror */
	if (lv_is_mirrored(lv) && !lp->region_size_supplied) {
		lp->region_size = first_seg(lv)->region_size;
		log_debug("Copying region size %s from existing mirror.",
			  display_size(lv->vg->cmd, lp->region_size));
	}

	/*
	 * Adjust log type
	 *
	 * If we are converting from a mirror to another mirror or simply
	 * changing the log type, we start by assuming they want the log
	 * type the same and then parse the given args.  OTOH, If we are
	 * converting from linear to mirror, then we start from the default
	 * position that the user would like a 'disk' log.
	 */
	*new_log_count = (*old_mimage_count > 1) ? *old_log_count : 1;
	if (!lp->corelog && !lp->mirrorlog)
		return 1;

	*new_log_count = arg_int_value(cmd, mirrorlog_ARG, lp->corelog ? MIRROR_LOG_CORE : DEFAULT_MIRRORLOG);

	log_verbose("Setting logging type to %s.", get_mirror_log_name(*new_log_count));

	/*
	 * Region size must not change on existing mirrors
	 */
	if (arg_is_set(cmd, regionsize_ARG) && lv_is_mirrored(lv) &&
	    (lp->region_size != first_seg(lv)->region_size)) {
		log_error("Mirror log region size cannot be changed on "
			  "an existing mirror.");
		return 0;
	}

	/*
	 * For the most part, we cannot handle multi-segment mirrors. Bail out
	 * early if we have encountered one.
	 */
	if (lv_is_mirrored(lv) && dm_list_size(&lv->segments) != 1) {
		log_error("Logical volume %s has multiple "
			  "mirror segments.", display_lvname(lv));
		return 0;
	}

	return 1;
}

/*
 * _lvconvert_mirrors_aux
 *
 * Add/remove mirror images and adjust log type.  'operable_pvs'
 * are the set of PVs open to removal or allocation - depending
 * on the operation being performed.
 */
static int _lvconvert_mirrors_aux(struct cmd_context *cmd,
				  struct logical_volume *lv,
				  struct lvconvert_params *lp,
				  struct dm_list *operable_pvs,
				  uint32_t new_mimage_count,
				  uint32_t new_log_count,
				  struct dm_list *pvh)
{
	uint32_t region_size;
	struct lv_segment *seg = first_seg(lv);
	struct logical_volume *layer_lv;
	uint32_t old_mimage_count = lv_mirror_count(lv);
	uint32_t old_log_count = _get_log_count(lv);

	if ((lp->mirrors == 1) && !lv_is_mirrored(lv)) {
		log_warn("WARNING: Logical volume %s is already not mirrored.",
			 display_lvname(lv));
		return 1;
	}

	if (!(region_size = adjusted_mirror_region_size(cmd, lv->vg->extent_size,
							lv->le_count,
							lp->region_size ? : seg->region_size, 0,
							vg_is_clustered(lv->vg))))
		return_0;

	if (lv_component_is_active(lv)) {
		log_error("Cannot convert logical volume %s with active component LV(s).",
			  display_lvname(lv));
		return 0;
	}

	if (!operable_pvs)
		operable_pvs = pvh;

	/*
	 * Up-convert from linear to mirror
	 */
	if (!lv_is_mirrored(lv)) {
		/* FIXME Share code with lvcreate */

		/*
		 * FIXME should we give not only pvh, but also all PVs
		 * currently taken by the mirror? Would make more sense from
		 * user perspective.
		 */
		if (!lv_add_mirrors(cmd, lv, new_mimage_count - 1, lp->stripes,
				    lp->stripe_size, region_size, new_log_count, operable_pvs,
				    lp->alloc, MIRROR_BY_LV))
			return_0;

		if (!arg_is_set(cmd, background_ARG))
			lp->need_polling = 1;

		goto out;
	}

	/*
	 * Up-convert m-way mirror to n-way mirror
	 */
	if (new_mimage_count > old_mimage_count) {
		if (lv_is_not_synced(lv)) {
			log_error("Can't add mirror to out-of-sync mirrored "
				  "LV: use lvchange --resync first.");
			return 0;
		}

		/*
		 * We allow snapshots of mirrors, but for now, we
		 * do not allow up converting mirrors that are under
		 * snapshots.  The layering logic is somewhat complex,
		 * and preliminary test show that the conversion can't
		 * seem to get the correct %'age of completion.
		 */
		if (lv_is_origin(lv)) {
			log_error("Can't add additional mirror images to "
				  "mirror %s which is under snapshots.",
				  display_lvname(lv));
			return 0;
		}

		/*
		 * Is there already a convert in progress?  We do not
		 * currently allow more than one.
		 */
		if (find_temporary_mirror(lv) || lv_is_converting(lv)) {
			log_error("%s is already being converted.  Unable to start another conversion.",
				  display_lvname(lv));
			return 0;
		}

		/*
		 * Log addition/removal should be done before the layer
		 * insertion to make the end result consistent with
		 * linear-to-mirror conversion.
		 */
		if (!_lv_update_log_type(cmd, lp, lv,
					 operable_pvs, new_log_count))
			return_0;

		/* Insert a temporary layer for syncing,
		 * only if the original lv is using disk log. */
		if (seg->log_lv && !_insert_lvconvert_layer(cmd, lv)) {
			log_error("Failed to insert resync layer.");
			return 0;
		}

		/* FIXME: can't have multiple mlogs. force corelog. */
		if (!lv_add_mirrors(cmd, lv,
				    new_mimage_count - old_mimage_count,
				    lp->stripes, lp->stripe_size,
				    region_size, 0U, operable_pvs, lp->alloc,
				    MIRROR_BY_LV)) {
			/* FIXME: failure inside library -> move error path processing into library. */
			layer_lv = seg_lv(first_seg(lv), 0);
			if (!remove_layer_from_lv(lv, layer_lv) ||
			    (lv_is_active(lv) && !deactivate_lv(cmd, layer_lv)) ||
			    !lv_remove(layer_lv) ||
			    !vg_write(lv->vg) || !vg_commit(lv->vg)) {
				log_error("ABORTING: Failed to remove "
					  "temporary mirror layer %s.",
					  display_lvname(layer_lv));
				log_error("Manual cleanup with vgcfgrestore "
					  "and dmsetup may be required.");
				return 0;
			}

			return_0;
		}
		if (seg->log_lv)
			lv->status |= CONVERTING;
		lp->need_polling = 1;

		goto out_skip_log_convert;
	}

	/*
	 * Down-convert (reduce # of mimages).
	 */
	if (new_mimage_count < old_mimage_count) {
		uint32_t nmc = old_mimage_count - new_mimage_count;
		uint32_t nlc = (!new_log_count || lp->mirrors == 1) ? 1U : 0U;

		/* FIXME: Why did nlc used to be calculated that way? */

		/* Reduce number of mirrors */
		if (lp->keep_mimages) {
			if (lp->track_changes) {
				log_error("--trackchanges is not available "
					  "to 'mirror' segment type.");
				return 0;
			}
			if (!lv_split_mirror_images(lv, lp->lv_split_name,
						    nmc, operable_pvs))
				return_0;
		} else if (!lv_remove_mirrors(cmd, lv, nmc, nlc,
					      is_mirror_image_removable, operable_pvs, 0))
			return_0;

		goto out; /* Just in case someone puts code between */
	}

out:
	/*
	 * Converting the log type
	 */
	if (lv_is_mirrored(lv) && (old_log_count != new_log_count)) {
		if (!_lv_update_log_type(cmd, lp, lv,
					 operable_pvs, new_log_count))
			return_0;
	}

out_skip_log_convert:

	if (!lv_update_and_reload(lv))
		return_0;

	return 1;
}

int mirror_remove_missing(struct cmd_context *cmd,
			  struct logical_volume *lv, int force)
{
	struct dm_list *failed_pvs;
	int log_count = _get_log_count(lv) - _failed_logs_count(lv);

	if (!(failed_pvs = _failed_pv_list(lv->vg)))
		return_0;

	if (force && _failed_mirrors_count(lv) == (int)lv_mirror_count(lv)) {
		log_error("No usable images left in %s.", display_lvname(lv));
		return lv_remove_with_dependencies(cmd, lv, DONT_PROMPT, 0);
	}

	/*
	 * We must adjust the log first, or the entire mirror
	 * will get stuck during a suspend.
	 */
	if (!_lv_update_mirrored_log(lv, failed_pvs, log_count))
		return_0;

	if (_failed_mirrors_count(lv) > 0 &&
	    !lv_remove_mirrors(cmd, lv, _failed_mirrors_count(lv),
			       log_count ? 0U : 1U,
			       _is_partial_lv, NULL, 0))
		return_0;

	if (lv_is_mirrored(lv) &&
	    !_lv_update_log_type(cmd, NULL, lv, failed_pvs, log_count))
		return_0;

	if (!lv_update_and_reload(lv))
		return_0;

	return 1;
}

/*
 * _lvconvert_mirrors_repair
 *
 * This function operates in two phases.  First, all of the bad
 * devices are removed from the mirror.  Then, if desired by the
 * user, the devices are replaced.
 *
 * 'old_mimage_count' and 'old_log_count' are there so we know
 * what to convert to after the removal of devices.
 */
static int _lvconvert_mirrors_repair(struct cmd_context *cmd,
				     struct logical_volume *lv,
				     struct lvconvert_params *lp,
				     struct dm_list *pvh)
{
	int failed_logs;
	int failed_mimages;
	int replace_logs = 0;
	int replace_mimages = 0;
	uint32_t log_count;

	uint32_t original_mimages = lv_mirror_count(lv);
	uint32_t original_logs = _get_log_count(lv);

	cmd->partial_activation = 1;
	lp->need_polling = 0;

	lv_check_transient(lv); /* TODO check this in lib for all commands? */

	if (!lv_is_partial(lv)) {
		log_print_unless_silent("Volume %s is consistent. Nothing to repair.",
					display_lvname(lv));
		return 1;
	}

	failed_mimages = _failed_mirrors_count(lv);
	failed_logs = _failed_logs_count(lv);

	/* Retain existing region size in case we need it later */
	if (!lp->region_size)
		lp->region_size = first_seg(lv)->region_size;

	if (!mirror_remove_missing(cmd, lv, 0))
		return_0;

	if (failed_mimages)
		log_print_unless_silent("Mirror status: %d of %d images failed.",
					failed_mimages, original_mimages);

	/*
	 * Count the failed log devices
	 */
	if (failed_logs)
		log_print_unless_silent("Mirror log status: %d of %d images failed.",
					failed_logs, original_logs);

	/*
	 * Find out our policies
	 */
	_lvconvert_mirrors_repair_ask(cmd, failed_logs, failed_mimages,
				      &replace_logs, &replace_mimages);

	/*
	 * Second phase - replace faulty devices
	 */
	lp->mirrors = replace_mimages ? original_mimages : (original_mimages - failed_mimages);

	/*
	 * It does not make sense to replace the log if the volume is no longer
	 * a mirror.
	 */
	if (lp->mirrors == 1)
		replace_logs = 0;

	log_count = replace_logs ? original_logs : (original_logs - failed_logs);

	while (replace_mimages || replace_logs) {
		log_warn("WARNING: Trying to up-convert to %d images, %d logs.", lp->mirrors, log_count);
		if (_lvconvert_mirrors_aux(cmd, lv, lp, NULL,
					   lp->mirrors, log_count, pvh))
			break;
		if (lp->mirrors > 2)
			--lp->mirrors;
		else if (log_count > 0)
			--log_count;
		else
			break; /* nowhere to go, anymore... */
	}

	if (replace_mimages && lv_mirror_count(lv) != original_mimages)
		log_warn("WARNING: Failed to replace %d of %d images in volume %s.",
			 original_mimages - lv_mirror_count(lv), original_mimages,
			 display_lvname(lv));
	if (replace_logs && _get_log_count(lv) != original_logs)
		log_warn("WARNING: Failed to replace %d of %d logs in volume %s.",
			 original_logs - _get_log_count(lv), original_logs,
			 display_lvname(lv));

	/* if (!arg_is_set(cmd, use_policies_ARG) && (lp->mirrors != old_mimage_count
						  || log_count != old_log_count))
						  return 0; */

	return 1;
}

static int _lvconvert_validate_thin(struct logical_volume *lv,
				    struct lvconvert_params *lp)
{
	if (!lv_is_thin_pool(lv) && !lv_is_thin_volume(lv))
		return 1;

	log_error("Converting thin%s segment type for %s to %s is not supported.",
		  lv_is_thin_pool(lv) ? " pool" : "",
		  display_lvname(lv), lp->segtype->name);

	if (lv_is_thin_volume(lv))
		return 0;

	/* Give advice for thin pool conversion */
	log_error("For pool data volume conversion use %s.",
		  display_lvname(seg_lv(first_seg(lv), 0)));
	log_error("For pool metadata volume conversion use %s.",
		  display_lvname(first_seg(lv)->metadata_lv));

	return 0;
}

/* Check for raid1 split trackchanges image to reject conversions on it. */
static int _raid_split_image_conversion(struct logical_volume *lv)
{
	const char *s;

	if (lv_is_raid_with_tracking(lv)) {
		log_error("Conversion of tracking raid1 LV %s is not supported.",
			  display_lvname(lv));
		return 1;
	}

	if (lv_is_raid_image(lv) &&
	    (s = strstr(lv->name, "_rimage_"))) {
		size_t len = s - lv->name;
		char raidlv_name[len + 1];
		const struct logical_volume *tmp_lv;

		strncpy(raidlv_name, lv->name, len);
		raidlv_name[len] = '\0';

		if (!(tmp_lv = find_lv(lv->vg, raidlv_name))) {
			log_error(INTERNAL_ERROR "Failed to find RaidLV of RAID subvolume %s.",
				  display_lvname(lv));
			return 1;
		}

		if (lv_is_raid_with_tracking(tmp_lv)) {
			log_error("Conversion of tracked raid1 subvolume %s is not supported.",
				  display_lvname(lv));
			return 1;
		}
	}

	return 0;
}

/*
 * _lvconvert_mirrors
 *
 * Determine what is being done.  Are we doing a conversion, repair, or
 * collapsing a stack?  Once determined, call helper functions.
 */
static int _lvconvert_mirrors(struct cmd_context *cmd,
			      struct logical_volume *lv,
			      struct lvconvert_params *lp)
{
	uint32_t old_mimage_count = 0;
	uint32_t old_log_count = 0;
	uint32_t new_mimage_count = 0;
	uint32_t new_log_count = 0;

	if (_raid_split_image_conversion(lv))
		return 0;

	if ((lp->corelog || lp->mirrorlog) && *lp->type_str && strcmp(lp->type_str, SEG_TYPE_NAME_MIRROR)) {
		log_error("--corelog and --mirrorlog are only compatible with mirror devices.");
		return 0;
	}

	if (!_lvconvert_validate_thin(lv, lp))
		return_0;

	if (lv_is_thin_type(lv)) {
		log_error("Mirror segment type cannot be used for thinpool%s.\n"
			  "Try \"%s\" segment type instead.",
			  lv_is_thin_pool_data(lv) ? "s" : " metadata",
			  SEG_TYPE_NAME_RAID1);
		return 0;
	}

	if (lv_is_cache_type(lv)) {
		log_error("Mirrors are not yet supported on cache LVs %s.",
			  display_lvname(lv));
		return 0;
	}

	if (_linear_type_requested(lp->type_str)) {
		if (arg_is_set(cmd, mirrors_ARG) && (arg_uint_value(cmd, mirrors_ARG, 0) != 0)) {
			log_error("Cannot specify mirrors with linear type.");
			return 0;
		}
		lp->mirrors_supplied = 1;
		lp->mirrors = 0;
	}

	/* Adjust mimage and/or log count */
	if (!_lvconvert_mirrors_parse_params(cmd, lv, lp,
					     &old_mimage_count, &old_log_count,
					     &new_mimage_count, &new_log_count))
		return_0;

	if (((old_mimage_count < new_mimage_count && old_log_count > new_log_count) ||
	     (old_mimage_count > new_mimage_count && old_log_count < new_log_count)) &&
	    lp->pv_count) {
		log_error("Cannot both allocate and free extents when "
			  "specifying physical volumes to use.");
		log_error("Please specify the operation in two steps.");
		return 0;
	}

	/* Nothing to do?  (Probably finishing collapse.) */
	if ((old_mimage_count == new_mimage_count) &&
	    (old_log_count == new_log_count))
		return 1;

	if (!_lvconvert_mirrors_aux(cmd, lv, lp, NULL,
				    new_mimage_count, new_log_count, lp->pvh))
		return_0;

	backup(lv->vg);

	if (!lp->need_polling)
		log_print_unless_silent("Logical volume %s converted.",
					display_lvname(lv));
	else
		log_print_unless_silent("Logical volume %s being converted.",
					display_lvname(lv));

	return 1;
}

static int _is_valid_raid_conversion(const struct segment_type *from_segtype,
				    const struct segment_type *to_segtype)
{
	if (!from_segtype)
		return 1;

	/* linear/striped/raid0 <-> striped/raid0/linear (restriping via raid) */
	if (segtype_is_striped(from_segtype) && segtype_is_striped(to_segtype))
		return 0;

	if (from_segtype == to_segtype)
		return 1;

	if (!segtype_is_raid(from_segtype) && !segtype_is_raid(to_segtype))
		return_0;  /* Not converting to or from RAID? */

	return 1;
}

/* Check for dm-raid target supporting raid4 conversion properly. */
static int _raid4_conversion_supported(struct logical_volume *lv, struct lvconvert_params *lp)
{
	int ret = 1;
	struct lv_segment *seg = first_seg(lv);

	if (seg_is_raid4(seg))
		ret = raid4_is_supported(lv->vg->cmd, seg->segtype);
	else if (segtype_is_raid4(lp->segtype))
		ret = raid4_is_supported(lv->vg->cmd, lp->segtype);

	if (ret)
		return 1;

	log_error("Cannot convert %s LV %s to %s.",
		  lvseg_name(seg), display_lvname(lv), lp->segtype->name);
	return 0;
}

static int _lvconvert_raid(struct logical_volume *lv, struct lvconvert_params *lp)
{
	int image_count = 0;
	struct cmd_context *cmd = lv->vg->cmd;
	struct lv_segment *seg = first_seg(lv);

	if (_raid_split_image_conversion(lv))
		return 0;

	if (_linear_type_requested(lp->type_str)) {
		if (arg_is_set(cmd, mirrors_ARG) && (arg_uint_value(cmd, mirrors_ARG, 0) != 0)) {
			log_error("Cannot specify mirrors with linear type.");
			return 0;
		}
		lp->mirrors_supplied = 1;
		lp->mirrors = 0;
	}

	if (!_lvconvert_validate_thin(lv, lp))
		return_0;

	if (!_is_valid_raid_conversion(seg->segtype, lp->segtype) &&
	    !lp->mirrors_supplied)
		goto try_new_takeover_or_reshape;

	if (seg_is_striped(seg) && !lp->mirrors_supplied)
		goto try_new_takeover_or_reshape;

	if (seg_is_linear(seg) && !lp->mirrors_supplied)
		goto try_new_takeover_or_reshape;

	/* Change number of RAID1 images */
	if (lp->mirrors_supplied || lp->keep_mimages) {
		image_count = lv_raid_image_count(lv);
		if (lp->mirrors_sign == SIGN_PLUS)
			image_count += lp->mirrors;
		else if (lp->mirrors_sign == SIGN_MINUS)
			image_count -= lp->mirrors;
		else
			image_count = lp->mirrors + 1;

		if (image_count < 1) {
			log_error("Unable to %s images by specified amount.",
				  lp->keep_mimages ? "split" : "reduce");
			return 0;
		}

		/* --trackchanges requires --splitmirrors which always has SIGN_MINUS */
		if (lp->track_changes && lp->mirrors != 1) {
			log_error("Exactly one image must be split off from %s when tracking changes.",
				  display_lvname(lv));
			return 0;
		}
	}

	if ((lp->corelog || lp->mirrorlog) && strcmp(lp->type_str, SEG_TYPE_NAME_MIRROR)) {
		log_error("--corelog and --mirrorlog are only compatible with mirror devices");
		return 0;
	}

	if (lp->track_changes)
		return lv_raid_split_and_track(lv, lp->yes, lp->pvh);

	if (lp->keep_mimages)
		return lv_raid_split(lv, lp->yes, lp->lv_split_name, image_count, lp->pvh);

	if (lp->mirrors_supplied) {
		if ((seg_is_striped(seg) && seg->area_count == 1) || seg_is_raid1(seg)) { /* ??? */
		if (!*lp->type_str || !strcmp(lp->type_str, SEG_TYPE_NAME_RAID1) || !strcmp(lp->type_str, SEG_TYPE_NAME_LINEAR) ||
		    (!strcmp(lp->type_str, SEG_TYPE_NAME_STRIPED) && image_count == 1)) {
			if (image_count > DEFAULT_RAID1_MAX_IMAGES) {
				log_error("Only up to %u mirrors in %s LV %s supported currently.",
					  DEFAULT_RAID1_MAX_IMAGES, lp->segtype->name, display_lvname(lv));
				return 0;
			}
			if (!lv_raid_change_image_count(lv, lp->yes, image_count,
							(lp->region_size_supplied || !seg->region_size) ?
							lp->region_size : seg->region_size , lp->pvh))
				return_0;

			log_print_unless_silent("Logical volume %s successfully converted.",
						display_lvname(lv));

			return 1;
		}
		}
		goto try_new_takeover_or_reshape;
	}

	if ((seg_is_linear(seg) || seg_is_striped(seg) || seg_is_mirrored(seg) || lv_is_raid(lv)) &&
	    (lp->type_str && lp->type_str[0])) {
		/* Activation is required later which precludes existing supported raid0 segment */
		if ((seg_is_any_raid0(seg) || segtype_is_any_raid0(lp->segtype)) &&
		    !(lp->target_attr & RAID_FEATURE_RAID0)) {
			log_error("RAID module does not support RAID0.");
			return 0;
		}

		/* Activation is required later which precludes existing supported raid4 segment */
		if (!_raid4_conversion_supported(lv, lp))
			return_0;

		/* Activation is required later which precludes existing supported raid10 segment */
		if ((seg_is_raid10(seg) || segtype_is_raid10(lp->segtype)) &&
		    !(lp->target_attr & RAID_FEATURE_RAID10)) {
			log_error("RAID module does not support RAID10.");
			return 0;
		}

		/* FIXME This needs changing globally. */
		if (!arg_is_set(cmd, stripes_long_ARG))
			lp->stripes = 0;
		if (!arg_is_set(cmd, type_ARG))
		       lp->segtype = NULL;
		if (!arg_is_set(cmd, regionsize_ARG))
		       lp->region_size = 0;

		if (!lv_raid_convert(lv, lp->segtype,
				     lp->yes, lp->force, lp->stripes, lp->stripe_size_supplied, lp->stripe_size,
				     lp->region_size, lp->pvh))
			return_0;

		log_print_unless_silent("Logical volume %s successfully converted.",
					display_lvname(lv));
		return 1;
	}

try_new_takeover_or_reshape:
	if (!_raid4_conversion_supported(lv, lp))
		return 0;

	/* FIXME This needs changing globally. */
	if (!arg_is_set(cmd, stripes_long_ARG))
		lp->stripes = 0;
	if (!arg_is_set(cmd, type_ARG))
	       lp->segtype = NULL;

	if (!lv_raid_convert(lv, lp->segtype,
			     lp->yes, lp->force, lp->stripes, lp->stripe_size_supplied, lp->stripe_size,
			     (lp->region_size_supplied || !seg->region_size) ?
			     lp->region_size : seg->region_size , lp->pvh))
		return_0;

	log_print_unless_silent("Logical volume %s successfully converted.",
				display_lvname(lv));
	return 1;
}

/*
 * Functions called to perform a specific operation on a specific LV type.
 *
 * _convert_<lvtype>_<operation>
 *
 * For cases where an operation does not apply to the LV itself, but
 * is implicitly redirected to a sub-LV, these functions locate the
 * correct sub-LV and call the operation on that sub-LV.  If a sub-LV
 * of the proper type is not found, these functions report the error.
 *
 * FIXME: the _lvconvert_foo() functions can be cleaned up since they
 * are now only called for valid combinations of LV type and operation.
 * After that happens, the code remaining in those functions can be
 * moved into the _convert_lvtype_operation() functions below.
 */

/*
 * Change the number of images in a mirror LV.
 * lvconvert --mirrors Number LV
 */
static int _convert_mirror_number(struct cmd_context *cmd, struct logical_volume *lv,
				  struct lvconvert_params *lp)
{
	return _lvconvert_mirrors(cmd, lv, lp);
}

/*
 * Split images from a mirror LV and use them to create a new LV.
 * lvconvert --splitmirrors Number LV
 *
 * Required options:
 * --name Name
 */

static int _convert_mirror_splitmirrors(struct cmd_context *cmd, struct logical_volume *lv,
					struct lvconvert_params *lp)
{
	return _lvconvert_mirrors(cmd, lv, lp);
}

/*
 * Change the type of log used by a mirror LV.
 * lvconvert --mirrorlog Type LV
 */
static int _convert_mirror_log(struct cmd_context *cmd, struct logical_volume *lv,
				  struct lvconvert_params *lp)
{
	return _lvconvert_mirrors(cmd, lv, lp);
}

/*
 * Convert mirror LV to linear LV.
 * lvconvert --type linear LV
 *
 * Alternate syntax:
 * lvconvert --mirrors 0 LV
 */
static int _convert_mirror_linear(struct cmd_context *cmd, struct logical_volume *lv,
				  struct lvconvert_params *lp)
{
	return _lvconvert_mirrors(cmd, lv, lp);
}

/*
 * Convert mirror LV to raid1 LV.
 * lvconvert --type raid1 LV
 */
static int _convert_mirror_raid(struct cmd_context *cmd, struct logical_volume *lv,
				struct lvconvert_params *lp)
{
	return _lvconvert_raid(lv, lp);
}

/*
 * Change the number of images in a raid1 LV.
 * lvconvert --mirrors Number LV
 */
static int _convert_raid_number(struct cmd_context *cmd, struct logical_volume *lv,
				struct lvconvert_params *lp)
{
	return _lvconvert_raid(lv, lp);
}

/*
 * Split images from a raid1 LV and use them to create a new LV.
 * lvconvert --splitmirrors Number LV
 *
 * Required options:
 * --trackchanges | --name Name
 */
static int _convert_raid_splitmirrors(struct cmd_context *cmd, struct logical_volume *lv,
				      struct lvconvert_params *lp)
{
	/* FIXME: split the splitmirrors section out of _lvconvert_raid and call it here. */
	return _lvconvert_raid(lv, lp);
}

/*
 * Convert a raid* LV to use a different raid level.
 * lvconvert --type raid* LV
 */
static int _convert_raid_raid(struct cmd_context *cmd, struct logical_volume *lv,
			      struct lvconvert_params *lp)
{
	return _lvconvert_raid(lv, lp);
}

/*
 * Convert a raid* LV to a mirror LV.
 * lvconvert --type mirror LV
 */
static int _convert_raid_mirror(struct cmd_context *cmd, struct logical_volume *lv,
			      struct lvconvert_params *lp)
{
	return _lvconvert_raid(lv, lp);
}

/*
 * Convert a raid* LV to a striped LV.
 * lvconvert --type striped LV
 */
static int _convert_raid_striped(struct cmd_context *cmd, struct logical_volume *lv,
				 struct lvconvert_params *lp)
{
	return _lvconvert_raid(lv, lp);
}

/*
 * Convert a raid* LV to a linear LV.
 * lvconvert --type linear LV
 */
static int _convert_raid_linear(struct cmd_context *cmd, struct logical_volume *lv,
				struct lvconvert_params *lp)
{
	return _lvconvert_raid(lv, lp);
}

/*
 * Convert a striped/linear LV to a mirror LV.
 * lvconvert --type mirror LV
 *
 * Required options:
 * --mirrors Number
 *
 * Alternate syntax:
 * This is equivalent to above when global/mirror_segtype_default="mirror".
 * lvconvert --mirrors Number LV
 */
static int _convert_striped_mirror(struct cmd_context *cmd, struct logical_volume *lv,
				   struct lvconvert_params *lp)
{
	return _lvconvert_mirrors(cmd, lv, lp);
}

/*
 * Convert a striped/linear LV to a raid* LV.
 * lvconvert --type raid* LV
 *
 * Required options:
 * --mirrors Number
 *
 * Alternate syntax:
 * This is equivalent to above when global/mirror_segtype_default="raid1".
 * lvconvert --mirrors Number LV
 */
static int _convert_striped_raid(struct cmd_context *cmd, struct logical_volume *lv,
				 struct lvconvert_params *lp)
{
	return _lvconvert_raid(lv, lp);
}

static int _convert_mirror(struct cmd_context *cmd, struct logical_volume *lv,
			   struct lvconvert_params *lp)
{
	if (arg_is_set(cmd, mirrors_ARG))
		return _convert_mirror_number(cmd, lv, lp);

	if (arg_is_set(cmd, splitmirrors_ARG))
		return _convert_mirror_splitmirrors(cmd, lv, lp);

	if (arg_is_set(cmd, mirrorlog_ARG) || arg_is_set(cmd, corelog_ARG))
		return _convert_mirror_log(cmd, lv, lp);

	if (_linear_type_requested(lp->type_str))
		return _convert_mirror_linear(cmd, lv, lp);

	if (segtype_is_raid(lp->segtype))
		return _convert_mirror_raid(cmd, lv, lp);

	log_error("Unknown operation on mirror LV %s.", display_lvname(lv));
	return 0;
}

static int _convert_raid(struct cmd_context *cmd, struct logical_volume *lv,
			 struct lvconvert_params *lp)
{
	if (arg_is_set(cmd, mirrors_ARG))
		return _convert_raid_number(cmd, lv, lp);

	if (arg_is_set(cmd, splitmirrors_ARG))
		return _convert_raid_splitmirrors(cmd, lv, lp);

	if (segtype_is_raid(lp->segtype))
		return _convert_raid_raid(cmd, lv, lp);

	if (segtype_is_mirror(lp->segtype))
		return _convert_raid_mirror(cmd, lv, lp);

	if (!strcmp(lp->type_str, SEG_TYPE_NAME_STRIPED))
		return _convert_raid_striped(cmd, lv, lp);

	if (_linear_type_requested(lp->type_str))
		return _convert_raid_linear(cmd, lv, lp);

	log_error("Unknown operation on raid LV %s.", display_lvname(lv));
	return 0;
}

static int _convert_striped(struct cmd_context *cmd, struct logical_volume *lv,
			    struct lvconvert_params *lp)
{
	const char *mirrors_type = find_config_tree_str(cmd, global_mirror_segtype_default_CFG, NULL);
	int raid_type = *lp->type_str && !strncmp(lp->type_str, "raid", 4);

	if (!raid_type) {
		if (!strcmp(lp->type_str, SEG_TYPE_NAME_MIRROR))
			return _convert_striped_mirror(cmd, lv, lp);

		/* --mirrors can mean --type mirror or --type raid1 depending on config setting. */

		if (arg_is_set(cmd, mirrors_ARG) && mirrors_type && !strcmp(mirrors_type, SEG_TYPE_NAME_MIRROR))
			return _convert_striped_mirror(cmd, lv, lp);
	}

	if (arg_is_set(cmd, mirrors_ARG) && mirrors_type && !strcmp(mirrors_type, SEG_TYPE_NAME_RAID1))
		return _convert_striped_raid(cmd, lv, lp);

	if (segtype_is_striped(lp->segtype) || segtype_is_raid(lp->segtype))
		return _convert_striped_raid(cmd, lv, lp);

	log_error("Unknown operation on striped or linear LV %s.", display_lvname(lv));
	return 0;
}

static int _lvconvert_raid_types(struct cmd_context *cmd, struct logical_volume *lv,
				 struct lvconvert_params *lp)
{
	struct lv_segment *seg = first_seg(lv);
	int ret = 0;

	/* If LV is inactive here, ensure it's not active elsewhere. */
	if (!lockd_lv(cmd, lv, "ex", 0))
		return_ECMD_FAILED;

	/* Set up segtype either from type_str or else to match the existing one. */
	if (!*lp->type_str)
		lp->segtype = seg->segtype;
	else if (!(lp->segtype = get_segtype_from_string(cmd, lp->type_str)))
		goto_out;

	if (!strcmp(lp->type_str, SEG_TYPE_NAME_MIRROR)) {
		if (!lp->mirrors_supplied && !seg_is_raid1(seg)) {
			log_error("Conversions to --type mirror require -m/--mirrors");
			goto out;
		}
	}

	/* lv->segtype can't be NULL */
	if (activation() && lp->segtype->ops->target_present &&
	    !lp->segtype->ops->target_present(cmd, NULL, &lp->target_attr)) {
		log_error("%s: Required device-mapper target(s) not "
			  "detected in your kernel.", lp->segtype->name);
		goto out;
	}

	/* Process striping parameters */
	/* FIXME This is incomplete */
	if (_mirror_or_raid_type_requested(cmd, lp->type_str) || _raid0_type_requested(lp->type_str) ||
	    _striped_type_requested(lp->type_str) || lp->mirrorlog || lp->corelog) {
		if (!arg_is_set(cmd, type_ARG))
			lp->segtype = first_seg(lv)->segtype;
		/* FIXME Handle +/- adjustments too? */
		if (!get_stripe_params(cmd, lp->segtype, &lp->stripes, &lp->stripe_size, &lp->stripes_supplied, &lp->stripe_size_supplied))
			goto_out;

		if (_raid0_type_requested(lp->type_str) || _striped_type_requested(lp->type_str))
			/* FIXME Shouldn't need to override get_stripe_params which defaults to 1 stripe (i.e. linear)! */
			/* The default keeps existing number of stripes, handled inside the library code */
			if (!arg_is_set(cmd, stripes_long_ARG))
				lp->stripes = 0;
	}

	if (lv_is_cache(lv))
		lv = seg_lv(first_seg(lv), 0);

	if (lv_is_mirror(lv)) {
		ret = _convert_mirror(cmd, lv, lp);
		goto out;
	}

	if (lv_is_raid(lv)) {
		ret = _convert_raid(cmd, lv, lp);
		goto out;
	}

	/*
	 * FIXME: add lv_is_striped() and lv_is_linear()?
	 * This does not include raid0 which is caught by the test above.
	 * If operations differ between striped and linear, split this case.
	 */
	if (segtype_is_striped(seg->segtype) || segtype_is_linear(seg->segtype)) {
		ret = _convert_striped(cmd, lv, lp);
		goto out;
	}

	/*
	 * The intention is to explicitly check all cases above and never
	 * reach here, but this covers anything that was missed.
	 */
	log_error("Cannot convert LV %s.", display_lvname(lv));

out:
	return ret ? ECMD_PROCESSED : ECMD_FAILED;
}

static int _lvconvert_splitsnapshot(struct cmd_context *cmd, struct logical_volume *cow)
{
	struct volume_group *vg = cow->vg;
	const char *cow_name = display_lvname(cow);

	if (lv_is_virtual_origin(origin_from_cow(cow))) {
		log_error("Unable to split off snapshot %s with virtual origin.", cow_name);
		return 0;
	}

	if (vg_is_shared(vg)) {
		/* FIXME: we need to create a lock for the new LV. */
		log_error("Unable to split snapshots in VG with lock_type %s.", vg->lock_type);
		return 0;
	}

	if (lv_is_active(cow)) {
		if (!lv_check_not_in_use(cow, 1))
			return_0;

		if ((arg_count(cmd, force_ARG) == PROMPT) &&
		    !arg_count(cmd, yes_ARG) &&
		    lv_is_visible(cow) &&
		    lv_is_active(cow)) {
			if (yes_no_prompt("Do you really want to split off active "
					  "logical volume %s? [y/n]: ", display_lvname(cow)) == 'n') {
				log_error("Logical volume %s not split.", display_lvname(cow));
				return 0;
			}
		}
	}

	if (!archive(vg))
		return_0;

	log_verbose("Splitting snapshot %s from its origin.", display_lvname(cow));

	if (!vg_remove_snapshot(cow))
		return_0;

	backup(vg);

	log_print_unless_silent("Logical Volume %s split from its origin.", display_lvname(cow));

	return 1;
}

static int _lvconvert_split_and_keep_cachepool(struct cmd_context *cmd,
				   struct logical_volume *lv,
				   struct logical_volume *cachepool_lv)
{
	log_debug("Detaching cache pool %s from cache LV %s.",
		  display_lvname(cachepool_lv), display_lvname(lv));

	if (!archive(lv->vg))
		return_0;

	if (!lv_cache_remove(lv))
		return_0;

	if (!vg_write(lv->vg) || !vg_commit(lv->vg))
		return_0;

	backup(lv->vg);

	log_print_unless_silent("Logical volume %s is not cached and cache pool %s is unused.",
				display_lvname(lv), display_lvname(cachepool_lv));

	return 1;
}

static int _lvconvert_split_and_remove_cachepool(struct cmd_context *cmd,
				   struct logical_volume *lv,
				   struct logical_volume *cachepool_lv)
{
	struct lv_segment *seg;
	struct logical_volume *remove_lv;

	seg = first_seg(lv);

	if (lv_is_partial(seg_lv(seg, 0))) {
		log_warn("WARNING: Cache origin logical volume %s is missing.",
			 display_lvname(seg_lv(seg, 0)));
		remove_lv = lv; /* When origin is missing, drop everything */
	} else
		remove_lv = seg->pool_lv;

	if (lv_is_partial(seg_lv(first_seg(seg->pool_lv), 0)))
		log_warn("WARNING: Cache pool data logical volume %s is missing.",
			 display_lvname(seg_lv(first_seg(seg->pool_lv), 0)));

	if (lv_is_partial(first_seg(seg->pool_lv)->metadata_lv))
		log_warn("WARNING: Cache pool metadata logical volume %s is missing.",
			 display_lvname(first_seg(seg->pool_lv)->metadata_lv));

	/* TODO: Check for failed cache as well to get prompting? */
	if (lv_is_partial(lv)) {
		if (first_seg(seg->pool_lv)->cache_mode != CACHE_MODE_WRITETHROUGH) {
			if (!arg_count(cmd, force_ARG)) {
				log_error("Conversion aborted.");
				log_error("Cannot uncache writethrough cache volume %s without --force.",
					  display_lvname(lv));
				return 0;
			}
			log_warn("WARNING: Uncaching of partially missing writethrough cache volume %s might destroy your data.",
				 display_lvname(lv));
		}

		if (!arg_count(cmd, yes_ARG) &&
		    yes_no_prompt("Do you really want to uncache %s with missing LVs? [y/n]: ",
				  display_lvname(lv)) == 'n') {
			log_error("Conversion aborted.");
			return 0;
		}
	}

	if (lvremove_single(cmd, remove_lv, NULL) != ECMD_PROCESSED)
		return_0;

	if (remove_lv != lv)
		log_print_unless_silent("Logical volume %s is not cached.", display_lvname(lv));

	return 1;
}

static int _lvconvert_snapshot(struct cmd_context *cmd,
			       struct logical_volume *lv,
			       const char *origin_name)
{
	struct logical_volume *org;
	const char *snap_name = display_lvname(lv);
	uint32_t chunk_size;
	int zero;

	if (strcmp(lv->name, origin_name) == 0) {
		log_error("Unable to use %s as both snapshot and origin.", snap_name);
		return 0;
	}

	chunk_size = arg_uint_value(cmd, chunksize_ARG, 8);
	if (chunk_size < 8 || chunk_size > 1024 || !is_power_of_2(chunk_size)) {
		log_error("Chunk size must be a power of 2 in the range 4K to 512K.");
		return 0;
	}

	if (!cow_has_min_chunks(lv->vg, lv->le_count, chunk_size))
		return_0;

	log_verbose("Setting chunk size to %s.", display_size(cmd, chunk_size));

	if (!(org = find_lv(lv->vg, origin_name))) {
		log_error("Couldn't find origin volume %s in Volume group %s.",
			  origin_name, lv->vg->name);
		return 0;
	}

	/*
	 * check_lv_rules() checks cannot be done via command definition
	 * rules because this LV is not processed by process_each_lv.
	 */

	/*
	 * check_lv_types() checks cannot be done via command definition
	 * LV_foo specification because this LV is not processed by process_each_lv.
	 */
	if (!validate_snapshot_origin(org))
		return_0;

	if (lv_component_is_active(org)) {
		log_error("Cannot use logical volume %s with active component LVs for snapshot origin.",
			  display_lvname(org));
		return 0;
	}

	log_warn("WARNING: Converting logical volume %s to snapshot exception store.",
		 snap_name);
	log_warn("THIS WILL DESTROY CONTENT OF LOGICAL VOLUME (filesystem etc.)");

	if (!arg_count(cmd, yes_ARG) &&
	    yes_no_prompt("Do you really want to convert %s? [y/n]: ",
			  snap_name) == 'n') {
		log_error("Conversion aborted.");
		return 0;
	}

	if (!deactivate_lv(cmd, lv)) {
		log_error("Couldn't deactivate logical volume %s.", snap_name);
		return 0;
	}

	if (first_seg(lv)->segtype->flags & SEG_CANNOT_BE_ZEROED)
		zero = 0;
	else
		zero = arg_int_value(cmd, zero_ARG, 1);

	if (!zero || !(lv->status & LVM_WRITE))
		log_warn("WARNING: %s not zeroed.", snap_name);
	else if (!activate_and_wipe_lv(lv, 0)) {
		log_error("Aborting. Failed to wipe snapshot exception store.");
		return 0;
	}

	if (!archive(lv->vg))
		return_0;

	if (!vg_add_snapshot(org, lv, NULL, org->le_count, chunk_size)) {
		log_error("Couldn't create snapshot.");
		return 0;
	}

	/* store vg on disk(s) */
	if (!lv_update_and_reload(org))
		return_0;

	log_print_unless_silent("Logical volume %s converted to snapshot.", snap_name);

	return 1;
}

static int _lvconvert_merge_old_snapshot(struct cmd_context *cmd,
					 struct logical_volume *lv,
					 struct logical_volume **lv_to_poll)
{
	int merge_on_activate = 0;
	struct logical_volume *origin = origin_from_cow(lv);
	struct lv_segment *snap_seg = find_snapshot(lv);
	struct lvinfo info;
	dm_percent_t snap_percent;

	/* Check if merge is possible */
	if (lv_is_merging_origin(origin)) {
		log_error("Cannot merge snapshot %s into the origin %s "
			  "with merging snapshot %s.",
			  display_lvname(lv), display_lvname(origin),
			  display_lvname(find_snapshot(origin)->lv));
		return 0;
	}

	if (lv_is_external_origin(origin_from_cow(lv))) {
		log_error("Cannot merge snapshot %s into "
			  "the read-only external origin %s.",
			  display_lvname(lv),
			  display_lvname(origin_from_cow(lv)));
		return 0;
	}

	/* FIXME: test when snapshot is remotely active */
	if (lv_info(cmd, lv, 0, &info, 1, 0)
	    && info.exists && info.live_table &&
	    (!lv_snapshot_percent(lv, &snap_percent) ||
	     snap_percent == DM_PERCENT_INVALID)) {
		log_error("Unable to merge invalidated snapshot LV %s.",
			  display_lvname(lv));
		return 0;
	}

	if (snap_seg->segtype->ops->target_present &&
	    !snap_seg->segtype->ops->target_present(cmd, snap_seg, NULL)) {
		log_error("Can't initialize snapshot merge. "
			  "Missing support in kernel?");
		return 0;
	}

	if (!archive(lv->vg))
		return_0;

	/*
	 * Prevent merge with open device(s) as it would likely lead
	 * to application/filesystem failure.  Merge on origin's next
	 * activation if either the origin or snapshot LV are currently
	 * open.
	 *
	 * FIXME testing open_count is racey; snapshot-merge target's
	 * constructor and DM should prevent appropriate devices from
	 * being open.
	 */
	if (lv_is_active(origin)) {
		if (!lv_check_not_in_use(origin, 0)) {
			log_print_unless_silent("Delaying merge since origin is open.");
			merge_on_activate = 1;
		} else if (!lv_check_not_in_use(lv, 0)) {
			log_print_unless_silent("Delaying merge since snapshot is open.");
			merge_on_activate = 1;
		}
	}

	init_snapshot_merge(snap_seg, origin);

	if (merge_on_activate) {
		/* Store and commit vg but skip starting the merge */
		if (!vg_write(lv->vg) || !vg_commit(lv->vg))
			return_0;
		backup(lv->vg);
	} else {
		/* Perform merge */
		if (!lv_update_and_reload(origin))
			return_0;

		if (!lv_has_target_type(origin->vg->vgmem, origin, NULL,
				       TARGET_NAME_SNAPSHOT_MERGE)) {
			/* Race during table reload prevented merging */
			merge_on_activate = 1;

		} else if (!lv_info(cmd, origin, 0, &info, 0, 0) || !info.exists) {
			log_print_unless_silent("Conversion starts after activation.");
			merge_on_activate = 1;
		} else {
			*lv_to_poll = origin;
		}
	}

	if (merge_on_activate)
		log_print_unless_silent("Merging of snapshot %s will occur on "
					"next activation of %s.",
					display_lvname(lv), display_lvname(origin));
	else
		log_print_unless_silent("Merging of volume %s started.",
					display_lvname(lv));

	return 1;
}

static int _lvconvert_merge_thin_snapshot(struct cmd_context *cmd,
					  struct logical_volume *lv)
{
	int origin_is_active = 0;
	struct lv_segment *snap_seg = first_seg(lv);
	struct logical_volume *origin = snap_seg->origin;

	if (!origin) {
		log_error("%s is not a mergeable logical volume.",
			  display_lvname(lv));
		return 0;
	}

	/* Check if merge is possible */
	if (lv_is_merging_origin(origin)) {
		log_error("Cannot merge snapshot %s into the origin %s "
			  "with merging snapshot %s.",
			  display_lvname(lv), display_lvname(origin),
			  display_lvname(find_snapshot(origin)->lv));
		return 0;
	}

	if (lv_is_external_origin(origin)) {
		if (!(origin = origin_from_cow(lv)))
			log_error(INTERNAL_ERROR "%s is missing origin.",
				  display_lvname(lv));
		else
			log_error("%s is read-only external origin %s.",
				  display_lvname(lv), display_lvname(origin));
		return 0;
	}

	if (lv_is_origin(origin)) {
		log_error("Merging into the old snapshot origin %s is not supported.",
			  display_lvname(origin));
		return 0;
	}

	if (!archive(lv->vg))
		return_0;

	/*
	 * Prevent merge with open device(s) as it would likely lead
	 * to application/filesystem failure.  Merge on origin's next
	 * activation if either the origin or snapshot LV can't be
	 * deactivated.
	 */
	if (!deactivate_lv(cmd, lv))
		log_print_unless_silent("Delaying merge since snapshot is open.");
	else if ((origin_is_active = lv_is_active(origin)) &&
		 !deactivate_lv(cmd, origin))
		log_print_unless_silent("Delaying merge since origin volume is open.");
	else {
		/*
		 * Both thin snapshot and origin are inactive,
		 * replace the origin LV with its snapshot LV.
		 */
		if (!thin_merge_finish(cmd, origin, lv))
			return_0;

		log_print_unless_silent("Volume %s replaced origin %s.",
					display_lvname(origin), display_lvname(lv));

		if (origin_is_active && !activate_lv(cmd, lv)) {
			log_error("Failed to reactivate origin %s.",
				  display_lvname(lv));
			return 0;
		}

		return 1;
	}

	init_snapshot_merge(snap_seg, origin);

	/* Commit vg, merge will start with next activation */
	if (!vg_write(lv->vg) || !vg_commit(lv->vg))
		return_0;

	log_print_unless_silent("Merging of thin snapshot %s will occur on "
				"next activation of %s.",
				display_lvname(lv), display_lvname(origin));
	backup(lv->vg);

	return 1;
}

static int _lvconvert_thin_pool_repair(struct cmd_context *cmd,
				       struct logical_volume *pool_lv,
				       struct dm_list *pvh, int poolmetadataspare)
{
	const char *dmdir = dm_dir();
	const char *thin_dump =
		find_config_tree_str_allow_empty(cmd, global_thin_dump_executable_CFG, NULL);
	const char *thin_repair =
		find_config_tree_str_allow_empty(cmd, global_thin_repair_executable_CFG, NULL);
	const struct dm_config_node *cn;
	const struct dm_config_value *cv;
	int ret = 0, status;
	int args = 0;
	const char *argv[MAX_PDATA_ARGS + 7]; /* Max supported args */
	char *dm_name, *trans_id_str;
	char meta_path[PATH_MAX];
	char pms_path[PATH_MAX];
	uint64_t trans_id;
	struct logical_volume *pmslv;
	struct logical_volume *mlv = first_seg(pool_lv)->metadata_lv;
	struct pipe_data pdata;
	FILE *f;

	if (!thin_repair || !thin_repair[0]) {
		log_error("Thin repair commnand is not configured. Repair is disabled.");
		return 0; /* Checking disabled */
	}

	pmslv = pool_lv->vg->pool_metadata_spare_lv;

	/* Check we have pool metadata spare LV */
	if (!handle_pool_metadata_spare(pool_lv->vg, 0, pvh, 1))
		return_0;

	if (pmslv != pool_lv->vg->pool_metadata_spare_lv) {
		if (!vg_write(pool_lv->vg) || !vg_commit(pool_lv->vg))
			return_0;
		pmslv = pool_lv->vg->pool_metadata_spare_lv;
	}

	if (!(dm_name = dm_build_dm_name(cmd->mem, mlv->vg->name,
					 mlv->name, NULL)) ||
	    (dm_snprintf(meta_path, sizeof(meta_path), "%s/%s", dmdir, dm_name) < 0)) {
		log_error("Failed to build thin metadata path.");
		return 0;
	}

	if (!(dm_name = dm_build_dm_name(cmd->mem, pmslv->vg->name,
					 pmslv->name, NULL)) ||
	    (dm_snprintf(pms_path, sizeof(pms_path), "%s/%s", dmdir, dm_name) < 0)) {
		log_error("Failed to build pool metadata spare path.");
		return 0;
	}

	if (!(cn = find_config_tree_array(cmd, global_thin_repair_options_CFG, NULL))) {
		log_error(INTERNAL_ERROR "Unable to find configuration for global/thin_repair_options");
		return 0;
	}

	for (cv = cn->v; cv && args < MAX_PDATA_ARGS; cv = cv->next) {
		if (cv->type != DM_CFG_STRING) {
			log_error("Invalid string in config file: "
				  "global/thin_repair_options");
			return 0;
		}
		argv[++args] = cv->v.str;
	}

	if (args >= MAX_PDATA_ARGS) {
		log_error("Too many options for thin repair command.");
		return 0;
	}

	argv[0] = thin_repair;
	argv[++args] = "-i";
	argv[++args] = meta_path;
	argv[++args] = "-o";
	argv[++args] = pms_path;
	argv[++args] = NULL;

	if (pool_is_active(pool_lv)) {
		log_error("Active pools cannot be repaired.  Use lvchange -an first.");
		return 0;
	}

	if (!activate_lv(cmd, pmslv)) {
		log_error("Cannot activate pool metadata spare volume %s.",
			  pmslv->name);
		return 0;
	}

	if (!activate_lv(cmd, mlv)) {
		log_error("Cannot activate thin pool metadata volume %s.",
			  mlv->name);
		goto deactivate_pmslv;
	}

	if (!(ret = exec_cmd(cmd, (const char * const *)argv, &status, 1))) {
		log_error("Repair of thin metadata volume of thin pool %s failed (status:%d). "
			  "Manual repair required!",
			  display_lvname(pool_lv), status);
		goto deactivate_mlv;
	}

	if (thin_dump[0]) {
		argv[0] = thin_dump;
		argv[1] = pms_path;
		argv[2] = NULL;

		if (!(f = pipe_open(cmd, argv, 0, &pdata)))
			log_warn("WARNING: Cannot read output from %s %s.", thin_dump, pms_path);
		else {
			/*
			 * Scan only the 1st. line for transation id.
			 * Watch out, if the thin_dump format changes
			 */
			if (fgets(meta_path, sizeof(meta_path), f) &&
			    (trans_id_str = strstr(meta_path, "transaction=\"")) &&
			    (sscanf(trans_id_str + 13, FMTu64, &trans_id) == 1) &&
			    (trans_id != first_seg(pool_lv)->transaction_id) &&
			    ((trans_id - 1) != first_seg(pool_lv)->transaction_id))
				log_error("Transaction id " FMTu64 " from pool \"%s/%s\" "
					  "does not match repaired transaction id "
					  FMTu64 " from %s.",
					  first_seg(pool_lv)->transaction_id,
					  pool_lv->vg->name, pool_lv->name, trans_id,
					  pms_path);

			(void) pipe_close(&pdata); /* killing pipe */
		}
	}

deactivate_mlv:
	if (!deactivate_lv(cmd, mlv)) {
		log_error("Cannot deactivate thin pool metadata volume %s.",
			  display_lvname(mlv));
		ret = 0;
	}

deactivate_pmslv:
	if (!deactivate_lv(cmd, pmslv)) {
		log_error("Cannot deactivate pool metadata spare volume %s.",
			  display_lvname(pmslv));
		ret = 0;
	}

	if (!ret)
		return 0;

	if (pmslv == pool_lv->vg->pool_metadata_spare_lv) {
		pool_lv->vg->pool_metadata_spare_lv = NULL;
		pmslv->status &= ~POOL_METADATA_SPARE;
		lv_set_visible(pmslv);
	}

	/* Try to allocate new pool metadata spare LV */
	if (!handle_pool_metadata_spare(pool_lv->vg, 0, pvh, poolmetadataspare))
		stack;

	if (dm_snprintf(meta_path, sizeof(meta_path), "%s_meta%%d", pool_lv->name) < 0) {
		log_error("Can't prepare new metadata name for %s.", pool_lv->name);
		return 0;
	}

	if (!generate_lv_name(pool_lv->vg, meta_path, pms_path, sizeof(pms_path))) {
		log_error("Can't generate new name for %s.", meta_path);
		return 0;
	}

	if (!detach_pool_metadata_lv(first_seg(pool_lv), &mlv))
		return_0;

	/* Swap _pmspare and _tmeta name */
	if (!swap_lv_identifiers(cmd, mlv, pmslv))
		return_0;

	if (!attach_pool_metadata_lv(first_seg(pool_lv), pmslv))
		return_0;

	/* Used _tmeta (now _pmspare) becomes _meta%d */
	if (!lv_rename_update(cmd, mlv, pms_path, 0))
		return_0;

	if (!vg_write(pool_lv->vg) || !vg_commit(pool_lv->vg))
		return_0;

	log_warn("WARNING: LV %s holds a backup of the unrepaired metadata. Use lvremove when no longer required.",
		 display_lvname(mlv));

	if (dm_list_size(&pool_lv->vg->pvs) > 1)
		log_warn("WARNING: New metadata LV %s might use different PVs.  Move it with pvmove if required.",
			 display_lvname(first_seg(pool_lv)->metadata_lv));

	return 1;
}

/* TODO: lots of similar code with  thinpool repair
 *       investigate possible better code sharing...
 */
static int _lvconvert_cache_repair(struct cmd_context *cmd,
				   struct logical_volume *cache_lv,
				   struct dm_list *pvh, int poolmetadataspare)
{
	const char *dmdir = dm_dir();
	const char *cache_repair =
		find_config_tree_str_allow_empty(cmd, global_cache_repair_executable_CFG, NULL);
	const struct dm_config_node *cn;
	const struct dm_config_value *cv;
	int ret = 0, status;
	int args = 0;
	const char *argv[MAX_PDATA_ARGS + 7]; /* Max supported args */
	char *dm_name;
	char meta_path[PATH_MAX];
	char pms_path[PATH_MAX];
	struct logical_volume *pool_lv;
	struct logical_volume *pmslv;
	struct logical_volume *mlv;

	pool_lv = lv_is_cache_pool(cache_lv) ? cache_lv : first_seg(cache_lv)->pool_lv;
	mlv = first_seg(pool_lv)->metadata_lv;

	if (!cache_repair || !cache_repair[0]) {
		log_error("Cache repair commnand is not configured. Repair is disabled.");
		return 0; /* Checking disabled */
	}

	pmslv = cache_lv->vg->pool_metadata_spare_lv;

	/* Check we have pool metadata spare LV */
	if (!handle_pool_metadata_spare(cache_lv->vg, 0, pvh, 1))
		return_0;

	if (pmslv != cache_lv->vg->pool_metadata_spare_lv) {
		if (!vg_write(cache_lv->vg) || !vg_commit(cache_lv->vg))
			return_0;
		pmslv = cache_lv->vg->pool_metadata_spare_lv;
	}

	if (!(dm_name = dm_build_dm_name(cmd->mem, mlv->vg->name,
					 mlv->name, NULL)) ||
	    (dm_snprintf(meta_path, sizeof(meta_path), "%s/%s", dmdir, dm_name) < 0)) {
		log_error("Failed to build cache metadata path.");
		return 0;
	}

	if (!(dm_name = dm_build_dm_name(cmd->mem, pmslv->vg->name,
					 pmslv->name, NULL)) ||
	    (dm_snprintf(pms_path, sizeof(pms_path), "%s/%s", dmdir, dm_name) < 0)) {
		log_error("Failed to build pool metadata spare path.");
		return 0;
	}

	if (!(cn = find_config_tree_array(cmd, global_cache_repair_options_CFG, NULL))) {
		log_error(INTERNAL_ERROR "Unable to find configuration for global/cache_repair_options");
		return 0;
	}

	for (cv = cn->v; cv && args < MAX_PDATA_ARGS; cv = cv->next) {
		if (cv->type != DM_CFG_STRING) {
			log_error("Invalid string in config file: "
				  "global/cache_repair_options");
			return 0;
		}
		argv[++args] = cv->v.str;
	}

	if (args >= MAX_PDATA_ARGS) {
		log_error("Too many options for cache repair command.");
		return 0;
	}

	argv[0] = cache_repair;
	argv[++args] = "-i";
	argv[++args] = meta_path;
	argv[++args] = "-o";
	argv[++args] = pms_path;
	argv[++args] = NULL;

	if (lv_is_active(cache_lv)) {
		log_error("Only inactive cache can be repaired.");
		return 0;
	}

	if (!activate_lv(cmd, pmslv)) {
		log_error("Cannot activate pool metadata spare volume %s.",
			  pmslv->name);
		return 0;
	}

	if (!activate_lv(cmd, mlv)) {
		log_error("Cannot activate cache pool metadata volume %s.",
			  mlv->name);
		goto deactivate_pmslv;
	}

	if (!(ret = exec_cmd(cmd, (const char * const *)argv, &status, 1))) {
		log_error("Repair of cache metadata volume of cache %s failed (status:%d). "
			  "Manual repair required!",
			  display_lvname(cache_lv), status);
		goto deactivate_mlv;
	}

	/* TODO: any active validation of cache-pool metadata? */

deactivate_mlv:
	if (!deactivate_lv(cmd, mlv)) {
		log_error("Cannot deactivate pool metadata volume %s.",
			  display_lvname(mlv));
		ret = 0;
	}

deactivate_pmslv:
	if (!deactivate_lv(cmd, pmslv)) {
		log_error("Cannot deactivate pool metadata spare volume %s.",
			  display_lvname(pmslv));
		ret = 0;
	}

	if (!ret)
		return 0;

	if (pmslv == cache_lv->vg->pool_metadata_spare_lv) {
		cache_lv->vg->pool_metadata_spare_lv = NULL;
		pmslv->status &= ~POOL_METADATA_SPARE;
		lv_set_visible(pmslv);
	}

	/* Try to allocate new pool metadata spare LV */
	if (!handle_pool_metadata_spare(cache_lv->vg, 0, pvh, poolmetadataspare))
		stack;

	if (dm_snprintf(meta_path, sizeof(meta_path), "%s_meta%%d", cache_lv->name) < 0) {
		log_error("Can't prepare new metadata name for %s.", cache_lv->name);
		return 0;
	}

	if (!generate_lv_name(cache_lv->vg, meta_path, pms_path, sizeof(pms_path))) {
		log_error("Can't generate new name for %s.", meta_path);
		return 0;
	}

	if (!detach_pool_metadata_lv(first_seg(pool_lv), &mlv))
		return_0;

	/* Swap _pmspare and _cmeta name */
	if (!swap_lv_identifiers(cmd, mlv, pmslv))
		return_0;

	if (!attach_pool_metadata_lv(first_seg(pool_lv), pmslv))
		return_0;

	/* Used _cmeta (now _pmspare) becomes _meta%d */
	if (!lv_rename_update(cmd, mlv, pms_path, 0))
		return_0;

	if (!vg_write(cache_lv->vg) || !vg_commit(cache_lv->vg))
		return_0;

	/* FIXME: just as with  thinpool repair - fix the warning
	 *        where moving doesn't make any sense (same disk storage)
         */
	log_warn("WARNING: If everything works, remove %s volume.",
		 display_lvname(mlv));

	log_warn("WARNING: Use pvmove command to move %s on the best fitting PV.",
		 display_lvname(first_seg(pool_lv)->metadata_lv));

	return 1;
}

static int _lvconvert_to_thin_with_external(struct cmd_context *cmd,
			   struct logical_volume *lv,
			   struct logical_volume *thinpool_lv)
{
	struct volume_group *vg = lv->vg;
	struct logical_volume *thin_lv;
	const char *origin_name;

	struct lvcreate_params lvc = {
		.activate = CHANGE_AEY,
		.alloc = ALLOC_INHERIT,
		.major = -1,
		.minor = -1,
		.suppress_zero_warn = 1, /* Suppress warning for this thin */
		.permission = LVM_READ,
		.pool_name = thinpool_lv->name,
		.pvh = &vg->pvs,
		.read_ahead = DM_READ_AHEAD_AUTO,
		.stripes = 1,
		.virtual_extents = lv->le_count,
	};

	if (_raid_split_image_conversion(lv))
		return 0;

	if (lv == thinpool_lv) {
		log_error("Can't use same LV %s for thin pool and thin volume.",
			  display_lvname(thinpool_lv));
		return 0;
	}

	if ((origin_name = arg_str_value(cmd, originname_ARG, NULL)))
		if (!validate_restricted_lvname_param(cmd, &vg->name, &origin_name))
			return_0;

	/*
	 * If NULL, an auto-generated 'lvol' name is used.
	 * If set, the lv create code checks the name isn't used.
	 */
	lvc.lv_name = origin_name;

	if (vg_is_shared(vg)) {
		/*
		 * FIXME: external origins don't work in lockd VGs.
		 * Prior to the lvconvert, there's a lock associated with
		 * the uuid of the external origin LV.  After the convert,
		 * that uuid belongs to the new thin LV, and a new LV with
		 * a new uuid exists as the non-thin, readonly external LV.
		 * We'd need to remove the lock for the previous uuid
		 * (the new thin LV will have no lock), and create a new
		 * lock for the new LV uuid used by the external LV.
		 */
		log_error("Can't use lock_type %s LV as external origin.",
			  vg->lock_type);
		return 0;
	}

	dm_list_init(&lvc.tags);

	if (!pool_supports_external_origin(first_seg(thinpool_lv), lv))
		return_0;

	if (!(lvc.segtype = get_segtype_from_string(cmd, SEG_TYPE_NAME_THIN)))
		return_0;

	if (!archive(vg))
		return_0;

	/*
	 * New thin LV needs to be created (all messages sent to pool) In this
	 * case thin volume is created READ-ONLY and also warn about not
	 * zeroing is suppressed.
	 *
	 * The new thin LV is created with the origin_name, or an autogenerated
	 * 'lvol' name.  Then the names and ids are swapped between the thin LV
	 * and the original/external LV.  So, the thin LV gets the name and id
	 * of the original LV arg, and the original LV arg gets the origin_name
	 * or the autogenerated name.
	 */

	if (!(thin_lv = lv_create_single(vg, &lvc)))
		return_0;

	if (!deactivate_lv(cmd, thin_lv)) {
		log_error("Aborting. Unable to deactivate new LV. "
			  "Manual intervention required.");
		return 0;
	}

	/*
	 * Crashing till this point will leave plain thin volume
	 * which could be easily removed by the user after i.e. power-off
	 */

	if (!swap_lv_identifiers(cmd, thin_lv, lv)) {
		stack;
		goto revert_new_lv;
	}

	/* Preserve read-write status of original LV here */
	thin_lv->status |= (lv->status & LVM_WRITE);

	if (!attach_thin_external_origin(first_seg(thin_lv), lv)) {
		stack;
		goto revert_new_lv;
	}

	if (!lv_update_and_reload(thin_lv)) {
		stack;
		goto deactivate_and_revert_new_lv;
	}

	log_print_unless_silent("Converted %s to thin volume with external origin %s.",
				display_lvname(thin_lv), display_lvname(lv));

	return 1;

deactivate_and_revert_new_lv:
	if (!swap_lv_identifiers(cmd, thin_lv, lv))
		stack;

	if (!deactivate_lv(cmd, thin_lv)) {
		log_error("Unable to deactivate failed new LV. "
			  "Manual intervention required.");
		return 0;
	}

	if (!detach_thin_external_origin(first_seg(thin_lv)))
		return_0;

revert_new_lv:
	/* FIXME Better to revert to backup of metadata? */
	if (!lv_remove(thin_lv) || !vg_write(vg) || !vg_commit(vg))
		log_error("Manual intervention may be required to remove "
			  "abandoned LV(s) before retrying.");
	else
		backup(vg);

	return 0;
}

static int _lvconvert_swap_pool_metadata(struct cmd_context *cmd,
					 struct logical_volume *lv,
					 struct logical_volume *metadata_lv)
{
	struct volume_group *vg = lv->vg;
	struct logical_volume *prev_metadata_lv;
	struct lv_segment *seg;
	struct lv_type *lvtype;
	char meta_name[NAME_LEN];
	const char *swap_name;
	uint32_t chunk_size;
	int is_thinpool;
	int is_cachepool;
	int lvt_enum;

	is_thinpool = lv_is_thin_pool(lv);
	is_cachepool = lv_is_cache_pool(lv);
	lvt_enum = get_lvt_enum(metadata_lv);
	lvtype = get_lv_type(lvt_enum);

	if (lvt_enum != striped_LVT && lvt_enum != linear_LVT && lvt_enum != raid_LVT) {
		log_error("LV %s with type %s cannot be used as a metadata LV.",
			  display_lvname(metadata_lv), lvtype ? lvtype->name : "unknown");
		return 0;
	}

	if (!lv_is_visible(metadata_lv)) {
		log_error("Can't convert internal LV %s.",
			  display_lvname(metadata_lv));
		return 0;
	}

	if (lv_is_locked(metadata_lv)) {
		log_error("Can't convert locked LV %s.",
			  display_lvname(metadata_lv));
		return 0;
	}

	if (lv_is_origin(metadata_lv) ||
	    lv_is_merging_origin(metadata_lv) ||
	    lv_is_external_origin(metadata_lv) ||
	    lv_is_virtual(metadata_lv)) {
		log_error("Pool metadata LV %s is of an unsupported type.",
			  display_lvname(metadata_lv));
		return 0;
	}

	/* FIXME cache pool */
	if (is_thinpool && pool_is_active(lv)) {
		/* If any volume referencing pool active - abort here */
		log_error("Cannot convert pool %s with active volumes.",
			  display_lvname(lv));
		return 0;
	}

	if ((dm_snprintf(meta_name, sizeof(meta_name), "%s%s", lv->name, is_cachepool ? "_cmeta" : "_tmeta") < 0)) {
                log_error("Failed to create internal lv names, pool name is too long.");
                return 0;
        }

	seg = first_seg(lv);

	/* Normally do NOT change chunk size when swapping */

	if (arg_is_set(cmd, chunksize_ARG)) {
		chunk_size = arg_uint_value(cmd, chunksize_ARG, 0);

		if ((chunk_size != seg->chunk_size) && !dm_list_empty(&lv->segs_using_this_lv)) {
			if (arg_count(cmd, force_ARG) == PROMPT) {
				log_error("Chunk size can be only changed with --force. Conversion aborted.");
				return 0;
			}

			if (!validate_pool_chunk_size(cmd, seg->segtype, chunk_size))
				return_0;

			log_warn("WARNING: Changing chunk size %s to %s for %s pool volume.",
				 display_size(cmd, seg->chunk_size),
				 display_size(cmd, chunk_size),
				 display_lvname(lv));

			/* Ok, user has likely some serious reason for this */
			if (!arg_count(cmd, yes_ARG) &&
			    yes_no_prompt("Do you really want to change chunk size for %s pool volume? [y/n]: ",
					  display_lvname(lv)) == 'n') {
				log_error("Conversion aborted.");
				return 0;
			}
		}

		seg->chunk_size = chunk_size;
	}

	if (!arg_count(cmd, yes_ARG) &&
	    yes_no_prompt("Do you want to swap metadata of %s pool with metadata volume %s? [y/n]: ",
			  display_lvname(lv),
			  display_lvname(metadata_lv)) == 'n') {
		log_error("Conversion aborted.");
		return 0;
	}

	if (!deactivate_lv(cmd, metadata_lv)) {
		log_error("Aborting. Failed to deactivate %s.",
			  display_lvname(metadata_lv));
		return 0;
	}

	if (!archive(vg))
		return_0;

	/* Swap names between old and new metadata LV */

	if (!detach_pool_metadata_lv(seg, &prev_metadata_lv))
		return_0;

	swap_name = metadata_lv->name;

	if (!lv_rename_update(cmd, metadata_lv, "pvmove_tmeta", 0))
		return_0;

	/* Give the previous metadata LV the name of the LV replacing it. */

	if (!lv_rename_update(cmd, prev_metadata_lv, swap_name, 0))
		return_0;

	/* Rename deactivated metadata LV to have _tmeta suffix */

	if (!lv_rename_update(cmd, metadata_lv, meta_name, 0))
		return_0;

	if (!attach_pool_metadata_lv(seg, metadata_lv))
		return_0;

	if (!vg_write(vg) || !vg_commit(vg))
		return_0;

	backup(vg);
	return 1;
}

/*
 * Create a new pool LV, using the lv arg as the data sub LV.
 * The metadata sub LV is either a new LV created here, or an
 * existing LV specified by --poolmetadata.
 *
 * process_single_lv is the LV currently being processed by
 * process_each_lv().  It will sometimes be the same as the
 * lv arg, and sometimes not.
 */

static int _lvconvert_to_pool(struct cmd_context *cmd,
			      struct logical_volume *lv,
			      struct logical_volume *process_single_lv,
			      int to_thinpool,
			      int to_cachepool,
			      struct dm_list *use_pvh)
{
	struct volume_group *vg = lv->vg;
	struct logical_volume *metadata_lv = NULL;  /* existing or created */
	struct logical_volume *data_lv;             /* lv arg renamed */
	struct logical_volume *pool_lv;             /* new lv created here */
	const char *pool_metadata_name;             /* user-specified lv name */
	const char *pool_name;                      /* name of original lv arg */
	char meta_name[NAME_LEN];                   /* generated sub lv name */
	char data_name[NAME_LEN];                   /* generated sub lv name */
	char converted_names[3*NAME_LEN];	    /* preserve names of converted lv */
	struct segment_type *pool_segtype;          /* thinpool or cachepool */
	struct lv_segment *seg;
	unsigned int target_attr = ~0;
	unsigned int activate_pool;
	unsigned int zero_metadata;
	uint64_t meta_size;
	uint32_t meta_extents;
	uint32_t chunk_size;
	int chunk_calc;
	cache_metadata_format_t cache_metadata_format;
	cache_mode_t cache_mode;
	const char *policy_name;
	struct dm_config_tree *policy_settings = NULL;
	int pool_metadata_spare;
	thin_discards_t discards;
	thin_zero_t zero_new_blocks;
	int r = 0;

	/* for handling lvmlockd cases */
	char *lockd_data_args = NULL;
	char *lockd_meta_args = NULL;
	char *lockd_data_name = NULL;
	char *lockd_meta_name = NULL;
	struct id lockd_data_id;
	struct id lockd_meta_id;
	const char *str_seg_type = to_cachepool ? SEG_TYPE_NAME_CACHE_POOL : SEG_TYPE_NAME_THIN_POOL;

	if (_raid_split_image_conversion(lv))
		return 0;

	if (lv_is_thin_pool(lv) || lv_is_cache_pool(lv)) {
		log_error(INTERNAL_ERROR "LV %s is already a pool.", display_lvname(lv));
		return 0;
	}

	pool_segtype = get_segtype_from_string(cmd, str_seg_type);

	if (!pool_segtype ||
	    !pool_segtype->ops->target_present(cmd, NULL, &target_attr)) {
		log_error("%s: Required device-mapper target(s) not detected in your kernel.",
			  str_seg_type);
		return 0;
	}

	/* Allow to have only thinpool active and restore it's active state. */
	activate_pool = to_thinpool && lv_is_active(lv);

	/* Wipe metadata_lv by default, but allow skipping this for cache pools. */
	zero_metadata = (to_cachepool) ? arg_int_value(cmd, zero_ARG, 1) : 1;

	/* An existing LV needs to have its lock freed once it becomes a data LV. */
	if (vg_is_shared(vg) && lv->lock_args) {
		lockd_data_args = dm_pool_strdup(cmd->mem, lv->lock_args);
		lockd_data_name = dm_pool_strdup(cmd->mem, lv->name);
		memcpy(&lockd_data_id, &lv->lvid.id[1], sizeof(struct id));
	}

	/*
	 * The internal LV names for pool data/meta LVs.
	 */

	if ((dm_snprintf(meta_name, sizeof(meta_name), "%s%s", lv->name, to_cachepool ? "_cmeta" : "_tmeta") < 0) ||
	    (dm_snprintf(data_name, sizeof(data_name), "%s%s", lv->name, to_cachepool ? "_cdata" : "_tdata") < 0)) {
		log_error("Failed to create internal lv names, pool name is too long.");
		return 0;
	}

	/* If LV is inactive here, ensure it's not active elsewhere. */
	if (!lockd_lv(cmd, lv, "ex", 0))
		return 0;

	/*
	 * If an existing LV is to be used as the metadata LV,
	 * verify that it's in a usable state.  These checks are
	 * not done by command def rules because this LV is not
	 * processed by process_each_lv.
	 */

	if ((pool_metadata_name = arg_str_value(cmd, poolmetadata_ARG, NULL))) {
		if (!validate_lvname_param(cmd, &vg->name, &pool_metadata_name)) {
			log_error("Metadata LV %s not found.", pool_metadata_name);
			return 0;
		}

		if (!(metadata_lv = find_lv(vg, pool_metadata_name))) {
			log_error("Unknown pool metadata LV %s.", pool_metadata_name);
			return 0;
		}

		/* An existing LV needs to have its lock freed once it becomes a meta LV. */
		if (vg_is_shared(vg) && metadata_lv->lock_args) {
			lockd_meta_args = dm_pool_strdup(cmd->mem, metadata_lv->lock_args);
			lockd_meta_name = dm_pool_strdup(cmd->mem, metadata_lv->name);
			memcpy(&lockd_meta_id, &metadata_lv->lvid.id[1], sizeof(struct id));
		}

		if (metadata_lv == lv) {
			log_error("Can't use same LV for pool data and metadata LV %s.",
				  display_lvname(metadata_lv));
			return 0;
		}

		if (metadata_lv == process_single_lv) {
			log_error("Use a different LV for pool metadata %s.",
				  display_lvname(metadata_lv));
			return 0;
		}

		if (!lv_is_visible(metadata_lv)) {
			log_error("Can't convert internal LV %s.",
				  display_lvname(metadata_lv));
			return 0;
		}

		if (lv_is_locked(metadata_lv)) {
			log_error("Can't convert locked LV %s.",
				  display_lvname(metadata_lv));
			return 0;
		}

		if (lv_is_mirror(metadata_lv)) {
			log_error("Mirror logical volumes cannot be used for pool metadata.");
			log_print_unless_silent("Try \"%s\" segment type instead.", SEG_TYPE_NAME_RAID1);
			return 0;
		}

		/* FIXME Tidy up all these type restrictions. */
		if (lv_is_cache_type(metadata_lv) ||
		    lv_is_thin_type(metadata_lv) ||
		    lv_is_cow(metadata_lv) || lv_is_merging_cow(metadata_lv) ||
		    lv_is_origin(metadata_lv) || lv_is_merging_origin(metadata_lv) ||
		    lv_is_external_origin(metadata_lv) ||
		    lv_is_virtual(metadata_lv)) {
			log_error("Pool metadata LV %s is of an unsupported type.",
				  display_lvname(metadata_lv));
			return 0;
		}

		/* If LV is inactive here, ensure it's not active elsewhere. */
		if (!lockd_lv(cmd, metadata_lv, "ex", 0))
			return 0;
	}

	if (!get_pool_params(cmd, pool_segtype,
			     &meta_size, &pool_metadata_spare,
			     &chunk_size, &discards, &zero_new_blocks))
		goto_bad;

	if (to_cachepool &&
	    !get_cache_params(cmd, &chunk_size, &cache_metadata_format, &cache_mode, &policy_name, &policy_settings))
		goto_bad;

	if (metadata_lv)
		meta_extents = metadata_lv->le_count;
	else if (meta_size)
		meta_extents = extents_from_size(cmd, meta_size, vg->extent_size);
	else
		meta_extents = 0; /* A default will be chosen by the "update" function. */

	/*
	 * Validate and/or choose defaults for meta_extents and chunk_size,
	 * this involves some complicated calculations.
	 */

	if (to_cachepool) {
		if (!update_cache_pool_params(cmd, vg->profile, vg->extent_size,
					      pool_segtype, target_attr,
					      lv->le_count,
					      &meta_extents,
					      &chunk_calc,
					      &chunk_size))
			goto_bad;
	} else {
		if (!update_thin_pool_params(cmd, vg->profile, vg->extent_size,
					     pool_segtype, target_attr,
					     lv->le_count,
					     &meta_extents,
					     &chunk_calc,
					     &chunk_size,
					     &discards, &zero_new_blocks))
			goto_bad;
	}

	if (metadata_lv && (meta_extents > metadata_lv->le_count)) {
		log_error("Pool metadata LV %s is too small (%u extents) for required metadata (%u extents).",
			  display_lvname(metadata_lv), metadata_lv->le_count, meta_extents);
		goto bad;
	}

	log_verbose("Pool metadata extents %u chunk_size %u", meta_extents, chunk_size);

	(void) dm_snprintf(converted_names, sizeof(converted_names), "%s%s%s",
			   display_lvname(lv),
			   metadata_lv ? " and " : "",
			   metadata_lv ? display_lvname(metadata_lv) : "");

	/*
	 * Verify that user wants to use these LVs.
	 */
	log_warn("WARNING: Converting %s to %s pool's data%s %s metadata wiping.",
		 converted_names,
		 to_cachepool ? "cache" : "thin",
		 metadata_lv ? " and metadata volumes" : " volume",
		 zero_metadata ? "with" : "WITHOUT");

	if (zero_metadata)
		log_warn("THIS WILL DESTROY CONTENT OF LOGICAL VOLUME (filesystem etc.)");
	else if (to_cachepool)
		log_warn("WARNING: Using mismatched cache pool metadata MAY DESTROY YOUR DATA!");

	if (!arg_count(cmd, yes_ARG) &&
	    yes_no_prompt("Do you really want to convert %s? [y/n]: ",
			  converted_names) == 'n') {
		log_error("Conversion aborted.");
		goto bad;
	}

	/*
	 * If a new metadata LV needs to be created, collect the settings for
	 * the new LV and create it.
	 *
	 * If an existing LV is used for metadata, deactivate/activate/wipe it.
	 */

	if (!metadata_lv) {
		uint32_t meta_stripes;
		uint32_t meta_stripe_size;
		uint32_t meta_readahead;
		alloc_policy_t meta_alloc;
		unsigned meta_stripes_supplied;
		unsigned meta_stripe_size_supplied;

		if (!get_stripe_params(cmd, get_segtype_from_string(cmd, SEG_TYPE_NAME_STRIPED),
				       &meta_stripes,
				       &meta_stripe_size,
				       &meta_stripes_supplied,
				       &meta_stripe_size_supplied))
			goto_bad;

		meta_readahead = arg_uint_value(cmd, readahead_ARG, cmd->default_settings.read_ahead);
		meta_alloc = (alloc_policy_t) arg_uint_value(cmd, alloc_ARG, ALLOC_INHERIT);

		if (!archive(vg))
			goto_bad;

		if (!(metadata_lv = alloc_pool_metadata(lv,
							meta_name,
							meta_readahead,
							meta_stripes,
							meta_stripe_size,
							meta_extents,
							meta_alloc,
							use_pvh)))
			goto_bad;
	} else {
		if (!deactivate_lv(cmd, metadata_lv)) {
			log_error("Aborting. Failed to deactivate %s.",
				  display_lvname(metadata_lv));
			goto bad;
		}

		if (!archive(vg))
			goto_bad;

		if (zero_metadata) {
			metadata_lv->status |= LV_ACTIVATION_SKIP;
			if (!activate_lv(cmd, metadata_lv)) {
				log_error("Aborting. Failed to activate metadata lv.");
				goto bad;
			}
			metadata_lv->status &= ~LV_ACTIVATION_SKIP;

			if (!wipe_lv(metadata_lv, (struct wipe_params) { .do_zero = 1 })) {
				log_error("Aborting. Failed to wipe metadata lv.");
				goto bad;
			}
		}
	}

	/*
	 * Deactivate the data LV and metadata LV.
	 * We are changing target type, so deactivate first.
	 */

	if (!deactivate_lv(cmd, metadata_lv)) {
		log_error("Aborting. Failed to deactivate metadata lv. "
			  "Manual intervention required.");
		goto bad;
	}

	if (!deactivate_lv(cmd, lv)) {
		log_error("Aborting. Failed to deactivate logical volume %s.",
			  display_lvname(lv));
		goto bad;
	}

	/*
	 * When the LV referenced by the original function arg "lv"
	 * is renamed, it is then referenced as "data_lv".
	 *
	 * pool_name    pool name taken from lv arg
	 * data_name    sub lv name, generated
	 * meta_name    sub lv name, generated
	 *
	 * pool_lv      new lv for pool object, created here
	 * data_lv      sub lv, was lv arg, now renamed
	 * metadata_lv  sub lv, existing or created here
	 */

	data_lv = lv;
	pool_name = lv->name; /* Use original LV name for pool name */

	/*
	 * Rename the original LV arg to the internal data LV naming scheme.
	 *
	 * Since we wish to have underlaying devs to match _[ct]data
	 * rename data LV to match pool LV subtree first,
	 * also checks for visible LV.
	 *
	 * FIXME: any more types prohibited here?
	 */

	if (!lv_rename_update(cmd, data_lv, data_name, 0))
		goto_bad;

	/*
	 * Create LV structures for the new pool LV object,
	 * and connect it to the data/meta LVs.
	 */

	if (!(pool_lv = lv_create_empty(pool_name, NULL,
					(to_cachepool ? CACHE_POOL : THIN_POOL) | VISIBLE_LV | LVM_READ | LVM_WRITE,
					ALLOC_INHERIT, vg))) {
		log_error("Creation of pool LV failed.");
		goto bad;
	}

	/* Allocate a new pool segment */
	if (!(seg = alloc_lv_segment(pool_segtype, pool_lv, 0, data_lv->le_count, 0,
				     pool_lv->status, 0, NULL, 1,
				     data_lv->le_count, 0, 0, 0, 0, NULL)))
		goto_bad;

	/* Add the new segment to the layer LV */
	dm_list_add(&pool_lv->segments, &seg->list);
	pool_lv->le_count = data_lv->le_count;
	pool_lv->size = data_lv->size;

	if (!attach_pool_data_lv(seg, data_lv))
		goto_bad;

	/*
	 * Create a new lock for a thin pool LV.  A cache pool LV has no lock.
	 * Locks are removed from existing LVs that are being converted to
	 * data and meta LVs (they are unlocked and deleted below.)
	 */
	if (vg_is_shared(vg)) {
		if (to_cachepool) {
			data_lv->lock_args = NULL;
			metadata_lv->lock_args = NULL;
		} else {
			data_lv->lock_args = NULL;
			metadata_lv->lock_args = NULL;

			if (!strcmp(vg->lock_type, "sanlock"))
				pool_lv->lock_args = "pending";
			else if (!strcmp(vg->lock_type, "dlm"))
				pool_lv->lock_args = "dlm";
			/* The lock_args will be set in vg_write(). */
		}
	}

	/* Apply settings to the new pool seg */
	if (to_cachepool) {
		if (!cache_set_params(seg, chunk_size, cache_metadata_format, cache_mode, policy_name, policy_settings))
			goto_bad;
	} else {
		seg->transaction_id = 0;
		seg->chunk_size = chunk_size;
		seg->discards = discards;
		seg->zero_new_blocks = zero_new_blocks;
	}

	/*
	 * Rename deactivated metadata LV to have _tmeta suffix.
	 * Implicit checks if metadata_lv is visible.
	 */
	if (pool_metadata_name &&
	    !lv_rename_update(cmd, metadata_lv, meta_name, 0))
		goto_bad;

	if (!attach_pool_metadata_lv(seg, metadata_lv))
		goto_bad;

	if (!handle_pool_metadata_spare(vg,
					metadata_lv->le_count,
					use_pvh, pool_metadata_spare))
		goto_bad;

	if (!vg_write(vg) || !vg_commit(vg))
		goto_bad;

	if (activate_pool && !lockd_lv(cmd, pool_lv, "ex", LDLV_PERSISTENT)) {
		log_error("Failed to lock pool LV %s.", display_lvname(pool_lv));
		goto out;
	}

	if (activate_pool &&
	    !activate_lv(cmd, pool_lv)) {
		log_error("Failed to activate pool logical volume %s.",
			  display_lvname(pool_lv));
		/* Deactivate subvolumes */
		if (!deactivate_lv(cmd, seg_lv(seg, 0)))
			log_error("Failed to deactivate pool data logical volume %s.",
				  display_lvname(seg_lv(seg, 0)));
		if (!deactivate_lv(cmd, seg->metadata_lv))
			log_error("Failed to deactivate pool metadata logical volume %s.",
				  display_lvname(seg->metadata_lv));
		goto out;
	}

	r = 1;

out:
	backup(vg);

	if (r)
		log_print_unless_silent("Converted %s to %s pool.",
					converted_names, to_cachepool ? "cache" : "thin");

	/*
	 * Unlock and free the locks from existing LVs that became pool data
	 * and meta LVs.
	 */
	if (lockd_data_name) {
		if (!lockd_lv_name(cmd, vg, lockd_data_name, &lockd_data_id, lockd_data_args, "un", LDLV_PERSISTENT))
			log_error("Failed to unlock pool data LV %s/%s", vg->name, lockd_data_name);
		lockd_free_lv(cmd, vg, lockd_data_name, &lockd_data_id, lockd_data_args);
	}

	if (lockd_meta_name) {
		if (!lockd_lv_name(cmd, vg, lockd_meta_name, &lockd_meta_id, lockd_meta_args, "un", LDLV_PERSISTENT))
			log_error("Failed to unlock pool metadata LV %s/%s", vg->name, lockd_meta_name);
		lockd_free_lv(cmd, vg, lockd_meta_name, &lockd_meta_id, lockd_meta_args);
	}
bad:
	if (policy_settings)
		dm_config_destroy(policy_settings);

	return r;
#if 0
revert_new_lv:
	/* TBD */
	if (!pool_metadata_lv_name) {
		if (!deactivate_lv(cmd, metadata_lv)) {
			log_error("Failed to deactivate metadata lv.");
			return 0;
		}
		if (!lv_remove(metadata_lv) || !vg_write(vg) || !vg_commit(vg))
			log_error("Manual intervention may be required to remove "
				  "abandoned LV(s) before retrying.");
		else
			backup(vg);
	}

	return 0;
#endif
}

static int _lvconvert_to_cache_vol(struct cmd_context *cmd,
				   struct logical_volume *lv,
				   struct logical_volume *cachepool_lv)
{
	struct logical_volume *cache_lv;
	uint32_t chunk_size = 0;
	cache_metadata_format_t cache_metadata_format;
	cache_mode_t cache_mode;
	const char *policy_name;
	struct dm_config_tree *policy_settings = NULL;
	int r = 0;

	if (_raid_split_image_conversion(lv))
		return 0;

	/* If LV is inactive here, ensure it's not active elsewhere. */
	if (!lockd_lv(cmd, lv, "ex", 0))
		return_0;

	if (!validate_lv_cache_create_pool(cachepool_lv))
		return_0;

	if (!get_cache_params(cmd, &chunk_size, &cache_metadata_format, &cache_mode, &policy_name, &policy_settings))
		goto_bad;

	if (!archive(lv->vg))
		goto_bad;

	if (!(cache_lv = lv_cache_create(cachepool_lv, lv)))
		goto_bad;

	if (!cache_set_params(first_seg(cache_lv), chunk_size, cache_metadata_format, cache_mode, policy_name, policy_settings))
		goto_bad;

	if (!lv_update_and_reload(cache_lv))
		goto_bad;

	log_print_unless_silent("Logical volume %s is now cached.",
				display_lvname(cache_lv));
	r = 1;
bad:
	if (policy_settings)
		dm_config_destroy(policy_settings);

	return r;
}

static struct convert_poll_id_list* _convert_poll_id_list_create(struct cmd_context *cmd,
								 const struct logical_volume *lv)
{
	struct convert_poll_id_list *idl = (struct convert_poll_id_list *) dm_pool_alloc(cmd->mem, sizeof(struct convert_poll_id_list));

	if (!idl) {
		log_error("Convert poll ID list allocation failed.");
		return NULL;
	}

	if (!(idl->id = _create_id(cmd, lv->vg->name, lv->name, lv->lvid.s))) {
		dm_pool_free(cmd->mem, idl);
		return_NULL;
	}

	idl->is_merging_origin = lv_is_merging_origin(lv);
	idl->is_merging_origin_thin = idl->is_merging_origin && seg_is_thin_volume(find_snapshot(lv));

	return idl;
}

/*
 * Data/results accumulated during processing.
 */
struct lvconvert_result {
	int need_polling;
	struct dm_list poll_idls;
};


/*
 * repair-related lvconvert utilities
 */

static int _lvconvert_repair_pvs_mirror(struct cmd_context *cmd, struct logical_volume *lv,
			struct processing_handle *handle,
			struct dm_list *use_pvh)
{
	struct lvconvert_result *lr = (struct lvconvert_result *) handle->custom_handle;
	struct lvconvert_params lp = { 0 };
	struct convert_poll_id_list *idl;
	struct lvinfo info;
	int ret;

	/*
	 * We want to allow cmirror active on multiple nodes to be repaired,
	 * but normal mirror to only be repaired if active exclusively here.
	 * If the LV is active it already has the necessary lock, but if not
	 * active, then require ex since we cannot know the active state on
	 * other hosts.
	 */
	if (!lv_is_active(lv)) {
		if (!lockd_lv(cmd, lv, "ex", 0))
			return_0;
	}

	/*
	 * FIXME: temporary use of lp because _lvconvert_mirrors_repair()
	 * and _aux() still use lp fields everywhere.
	 * Migrate them away from using lp (for the most part just use
	 * local variables, and check arg_values directly).
	 */

	/*
	 * Fill in any lp fields here that this fn expects to be set before
	 * it's called.  It's hard to tell what the old code expects in lp
	 * for repair; it doesn't take the stripes option, but it seems to
	 * expect lp.stripes to be set to 1.
	 */
	lp.alloc = (alloc_policy_t) arg_uint_value(cmd, alloc_ARG, ALLOC_INHERIT);
	lp.stripes = 1;

	ret = _lvconvert_mirrors_repair(cmd, lv, &lp, use_pvh);

	if (lp.need_polling) {
		if (!lv_info(cmd, lv, 0, &info, 0, 0) || !info.exists)
			log_print_unless_silent("Conversion starts after activation.");
		else {
			if (!(idl = _convert_poll_id_list_create(cmd, lv)))
				return 0;
			dm_list_add(&lr->poll_idls, &idl->list);
		}
		lr->need_polling = 1;
	}

	return ret;
}

static void _lvconvert_repair_pvs_raid_ask(struct cmd_context *cmd, int *do_it)
{
	const char *dev_policy;

	*do_it = 1;

	if (arg_is_set(cmd, usepolicies_ARG)) {
		dev_policy = find_config_tree_str(cmd, activation_raid_fault_policy_CFG, NULL);

		if (!strcmp(dev_policy, "allocate") ||
		    !strcmp(dev_policy, "replace"))
			return;

		/* else if (!strcmp(dev_policy, "anything_else")) -- no replace */
		*do_it = 0;
		return;
	}

	if (!arg_count(cmd, yes_ARG) &&
	    yes_no_prompt("Attempt to replace failed RAID images "
			  "(requires full device resync)? [y/n]: ") == 'n') {
		*do_it = 0;
	}
}

static int _lvconvert_repair_pvs_raid(struct cmd_context *cmd, struct logical_volume *lv,
			struct processing_handle *handle,
			struct dm_list *use_pvh)
{
	struct dm_list *failed_pvs;
	int do_it;

	if (!lv_is_active(lv_lock_holder(lv))) {
		log_error("%s must be active to perform this operation.", display_lvname(lv));
		return 0;
	}

	lv_check_transient(lv); /* TODO check this in lib for all commands? */

	_lvconvert_repair_pvs_raid_ask(cmd, &do_it);

	if (do_it) {
		if (!(failed_pvs = _failed_pv_list(lv->vg)))
			return_0;

		if (!lv_raid_replace(lv, arg_count(cmd, force_ARG), failed_pvs, use_pvh)) {
			log_error("Failed to replace faulty devices in %s.",
				  display_lvname(lv));
			return 0;
		}

		log_print_unless_silent("Faulty devices in %s successfully replaced.",
					display_lvname(lv));
		return 1;
	}

	/* "warn" if policy not set to replace */
	if (arg_is_set(cmd, usepolicies_ARG))
		log_warn("Use 'lvconvert --repair %s' to replace "
			 "failed device.", display_lvname(lv));
	return 1;
}

static int _lvconvert_repair_pvs(struct cmd_context *cmd, struct logical_volume *lv,
			struct processing_handle *handle)
{
	struct dm_list *failed_pvs;
	struct dm_list *use_pvh;
	int ret;

	if (cmd->position_argc > 1) {
		/* First pos arg is required LV, remaining are optional PVs. */
		if (!(use_pvh = create_pv_list(cmd->mem, lv->vg, cmd->position_argc - 1, cmd->position_argv + 1, 0)))
			return_ECMD_FAILED;
	} else
		use_pvh = &lv->vg->pvs;

	if (lv_is_raid(lv))
		ret = _lvconvert_repair_pvs_raid(cmd, lv, handle, use_pvh);
	else if (lv_is_mirror(lv))
		ret = _lvconvert_repair_pvs_mirror(cmd, lv, handle, use_pvh);
	else
		ret = 0;

	if (ret && arg_is_set(cmd, usepolicies_ARG)) {
		if ((failed_pvs = _failed_pv_list(lv->vg)))
			_remove_missing_empty_pv(lv->vg, failed_pvs);
	}

	return ret ? ECMD_PROCESSED : ECMD_FAILED;
}

static int _lvconvert_repair_cachepool_thinpool(struct cmd_context *cmd, struct logical_volume *lv,
			struct processing_handle *handle)
{
	int poolmetadataspare = arg_int_value(cmd, poolmetadataspare_ARG, DEFAULT_POOL_METADATA_SPARE);
	struct dm_list *use_pvh;

	/* ensure it's not active elsewhere. */
	if (!lockd_lv(cmd, lv, "ex", 0))
		return_0;

	if (cmd->position_argc > 1) {
		/* First pos arg is required LV, remaining are optional PVs. */
		if (!(use_pvh = create_pv_list(cmd->mem, lv->vg, cmd->position_argc - 1, cmd->position_argv + 1, 0)))
			return_ECMD_FAILED;
	} else
		use_pvh = &lv->vg->pvs;

	if (lv_is_thin_pool(lv)) {
		if (!_lvconvert_thin_pool_repair(cmd, lv, use_pvh, poolmetadataspare))
			return_ECMD_FAILED;
	} else /* cache */ {
		if (!_lvconvert_cache_repair(cmd, lv, use_pvh, poolmetadataspare))
			return_ECMD_FAILED;
	}

	return ECMD_PROCESSED;
}

static int _lvconvert_repair_single(struct cmd_context *cmd, struct logical_volume *lv,
			struct processing_handle *handle)
{
	if (lv_is_thin_pool(lv) ||
	    lv_is_cache(lv) ||
	    lv_is_cache_pool(lv))
		return _lvconvert_repair_cachepool_thinpool(cmd, lv, handle);

	if (lv_is_raid(lv) || lv_is_mirror(lv))
		return _lvconvert_repair_pvs(cmd, lv, handle);

	log_error("Unsupported volume type for repair of volume %s.",
		  display_lvname(lv));

	return ECMD_FAILED;
}

/*
 * FIXME: add option --repair-pvs to call _lvconvert_repair_pvs() directly,
 * and option --repair-thinpool to call _lvconvert_repair_thinpool().
 * and option --repair-cache to call _lvconvert_repair_cache().
 * and option --repair-cachepool to call _lvconvert_repair_cachepool().
 */
int lvconvert_repair_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	struct processing_handle *handle;
	struct lvconvert_result lr = { 0 };
	struct convert_poll_id_list *idl;
	int saved_ignore_suspended_devices;
	int ret, poll_ret;

	dm_list_init(&lr.poll_idls);

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &lr;

	saved_ignore_suspended_devices = ignore_suspended_devices();
	init_ignore_suspended_devices(1);

	cmd->handles_missing_pvs = 1;

	ret = process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			      handle, NULL, &_lvconvert_repair_single);

	init_ignore_suspended_devices(saved_ignore_suspended_devices);

	if (lr.need_polling) {
		dm_list_iterate_items(idl, &lr.poll_idls) {
			poll_ret = _lvconvert_poll_by_id(cmd, idl->id,
						arg_is_set(cmd, background_ARG), 0, 0);
			if (poll_ret > ret)
				ret = poll_ret;
		}
	}

	destroy_processing_handle(cmd, handle);

	return ret;
}

static int _lvconvert_replace_pv_single(struct cmd_context *cmd, struct logical_volume *lv,
			struct processing_handle *handle)
{
	struct arg_value_group_list *group;
	const char *tmp_str;
	struct dm_list *use_pvh;
	struct dm_list *replace_pvh;
	char **replace_pvs;
	int replace_pv_count;
	int i;

	if (cmd->position_argc > 1) {
		/* First pos arg is required LV, remaining are optional PVs. */
		if (!(use_pvh = create_pv_list(cmd->mem, lv->vg, cmd->position_argc - 1, cmd->position_argv + 1, 0)))
			return_ECMD_FAILED;
	} else
		use_pvh = &lv->vg->pvs;

	if (!(replace_pv_count = arg_count(cmd, replace_ARG)))
		return_ECMD_FAILED;

	if (!(replace_pvs = dm_pool_alloc(cmd->mem, sizeof(char *) * replace_pv_count)))
		return_ECMD_FAILED;

	i = 0;
	dm_list_iterate_items(group, &cmd->arg_value_groups) {
		if (!grouped_arg_is_set(group->arg_values, replace_ARG))
			continue;
		if (!(tmp_str = grouped_arg_str_value(group->arg_values, replace_ARG, NULL))) {
			log_error("Failed to get '--replace' argument");
			return_ECMD_FAILED;
		}
		if (!(replace_pvs[i++] = dm_pool_strdup(cmd->mem, tmp_str)))
			return_ECMD_FAILED;
	}

	if (!(replace_pvh = create_pv_list(cmd->mem, lv->vg, replace_pv_count, replace_pvs, 0)))
		return_ECMD_FAILED;

	if (!lv_raid_replace(lv, arg_count(cmd, force_ARG), replace_pvh, use_pvh))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

int lvconvert_replace_pv_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	struct processing_handle *handle;
	struct lvconvert_result lr = { 0 };
	int ret;

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &lr;

	ret = process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			      handle, NULL, &_lvconvert_replace_pv_single);

	destroy_processing_handle(cmd, handle);

	return ret;
}


/*
 * Merge a COW snapshot LV into its origin.
 */

static int _lvconvert_merge_snapshot_single(struct cmd_context *cmd,
                                       struct logical_volume *lv,
                                       struct processing_handle *handle)
{
	struct lvconvert_result *lr = (struct lvconvert_result *) handle->custom_handle;
	struct logical_volume *lv_to_poll = NULL;
	struct convert_poll_id_list *idl;

	if (!_lvconvert_merge_old_snapshot(cmd, lv, &lv_to_poll))
		return_ECMD_FAILED;

	if (lv_to_poll) {
		if (!(idl = _convert_poll_id_list_create(cmd, lv_to_poll)))
			return_ECMD_FAILED;
		dm_list_add(&lr->poll_idls, &idl->list);
		lr->need_polling = 1;
	}

	return ECMD_PROCESSED;
}

int lvconvert_merge_snapshot_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	struct processing_handle *handle;
	struct lvconvert_result lr = { 0 };
	struct convert_poll_id_list *idl;
	int ret, poll_ret;

	dm_list_init(&lr.poll_idls);

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &lr;

	ret = process_each_lv(cmd, cmd->position_argc, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			      handle, NULL, &_lvconvert_merge_snapshot_single);

	if (lr.need_polling) {
		dm_list_iterate_items(idl, &lr.poll_idls) {
			poll_ret = _lvconvert_poll_by_id(cmd, idl->id,
						arg_is_set(cmd, background_ARG), 1, 0);
			if (poll_ret > ret)
				ret = poll_ret;
		}
	}

	destroy_processing_handle(cmd, handle);

	return ret;
}

/*
 * Separate a COW snapshot from its origin.
 *
 * lvconvert --splitsnapshot LV_snapshot
 * lvconvert_split_cow_snapshot
 */

static int _lvconvert_split_snapshot_single(struct cmd_context *cmd,
                                       struct logical_volume *lv,
                                       struct processing_handle *handle)
{
	if (!_lvconvert_splitsnapshot(cmd, lv))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

int lvconvert_split_snapshot_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	return process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, NULL, &_lvconvert_split_snapshot_single);
}

/*
 * Combine two LVs that were once an origin/cow pair of LVs, were then
 * separated with --splitsnapshot, and now with this command are combined again
 * into the origin/cow pair.
 *
 * This is an obscure command that has little to no real uses.
 *
 * The command has unusual handling of position args.  The first position arg
 * will become the origin LV, and is not processed by process_each_lv.  The
 * second position arg will become the cow LV and is processed by
 * process_each_lv.
 *
 * The single function can grab the origin LV from position_argv[0].
 *
 * begin with an ordinary LV foo:
 * lvcreate -n foo -L 1 vg
 *
 * create a cow snapshot of foo named foosnap:
 * lvcreate -s -L 1 -n foosnap vg/foo
 *
 * now, foo is an "origin LV" and foosnap is a "cow LV"
 * (foosnap matches LV_snapshot aka lv_is_cow)
 *
 * split the two LVs apart:
 * lvconvert --splitsnapshot vg/foosnap
 *
 * now, foo is *not* an origin LV and foosnap is *not* a cow LV
 * (foosnap does not match LV_snapshot)
 *
 * now, combine the two LVs again:
 * lvconvert --snapshot vg/foo vg/foosnap
 *
 * after this, foosnap will match LV_snapshot again.
 *
 * FIXME: when splitsnapshot is run, the previous cow LV should be
 * flagged in the metadata somehow, and then that flag should be
 * required here.  As it is now, the first and second args
 * (origin and cow) can be swapped and nothing catches it.
 */

static int _lvconvert_combine_split_snapshot_single(struct cmd_context *cmd,
                                       struct logical_volume *lv,
                                       struct processing_handle *handle)
{
	const char *origin_name = cmd->position_argv[0];

	if (vg_is_shared(lv->vg)) {
		log_error("Unable to combine split snapshots in VG with lock_type %s", lv->vg->lock_type);
		return ECMD_FAILED;
	}

	/* If origin_name includes VG name, the VG name is removed. */
	if (!validate_lvname_param(cmd, &lv->vg->name, &origin_name))
		return_ECMD_FAILED;

	if (!_lvconvert_snapshot(cmd, lv, origin_name))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

int lvconvert_combine_split_snapshot_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	const char *vgname = NULL;
	const char *lvname1_orig;
	const char *lvname2_orig;
	const char *lvname1_split;
	char *vglv;
	int vglv_sz;

	/*
	 * Hack to accomodate an old parsing quirk that allowed the
	 * the VG name to be attached to only the LV in arg pos 1,
	 * i.e. lvconvert -s vgname/lvname lvname
	 *
	 * The LV name in arg pos 2 is the one that is processed
	 * by process_each_lv().  If that LV has no VG name, but
	 * the first LV does, then copy the VG name from arg pos 1
	 * and add it to the LV name in arg pos 2 so that the
	 * standard arg parsing in process_each_lv will find it.
	 *
	 * This is the only instance in all commands.
	 */

	lvname1_orig = cmd->position_argv[0];
	lvname2_orig = cmd->position_argv[1];

	if (strchr(lvname1_orig, '/') && !strchr(lvname2_orig, '/') && !getenv("LVM_VG_NAME")) {
		if (!(lvname1_split = dm_pool_strdup(cmd->mem, lvname1_orig)))
			return_ECMD_FAILED;

		if (!validate_lvname_param(cmd, &vgname, &lvname1_split))
			return_ECMD_FAILED;

		vglv_sz = strlen(vgname) + strlen(lvname2_orig) + 2;
		if (!(vglv = dm_pool_alloc(cmd->mem, vglv_sz)) ||
		    dm_snprintf(vglv, vglv_sz, "%s/%s", vgname, lvname2_orig) < 0) {
       			log_error("vg/lv string alloc failed.");
			return_ECMD_FAILED;
		}

		/* vglv is now vgname/lvname2 and replaces lvname2_orig */

		cmd->position_argv[1] = vglv;
	}

	return process_each_lv(cmd, 1, cmd->position_argv + 1, NULL, NULL, READ_FOR_UPDATE,
			       NULL, NULL, &_lvconvert_combine_split_snapshot_single);
}

static int _lvconvert_start_poll_single(struct cmd_context *cmd,
                                       struct logical_volume *lv,
                                       struct processing_handle *handle)
{
	struct lvconvert_result *lr = (struct lvconvert_result *) handle->custom_handle;
	struct convert_poll_id_list *idl;

	if (!(idl = _convert_poll_id_list_create(cmd, lv)))
		return_ECMD_FAILED;
	dm_list_add(&lr->poll_idls, &idl->list);

	lr->need_polling = 1;

	return ECMD_PROCESSED;
}

int lvconvert_start_poll_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	struct processing_handle *handle;
	struct lvconvert_result lr = { 0 };
	struct convert_poll_id_list *idl;
	int saved_ignore_suspended_devices;
	int ret, poll_ret;

	dm_list_init(&lr.poll_idls);

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &lr;

	saved_ignore_suspended_devices = ignore_suspended_devices();
	init_ignore_suspended_devices(1);

	cmd->handles_missing_pvs = 1;

	ret = process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			      handle, NULL, &_lvconvert_start_poll_single);

	init_ignore_suspended_devices(saved_ignore_suspended_devices);

	if (lr.need_polling) {
		dm_list_iterate_items(idl, &lr.poll_idls) {
			poll_ret = _lvconvert_poll_by_id(cmd, idl->id,
						arg_is_set(cmd, background_ARG), 0, 0);
			if (poll_ret > ret)
				ret = poll_ret;
		}
	}

	destroy_processing_handle(cmd, handle);

	return ret;
}

static int _lvconvert_to_pool_single(struct cmd_context *cmd,
					 struct logical_volume *lv,
					 struct processing_handle *handle)
{
	struct dm_list *use_pvh = NULL;
	int to_thinpool = 0;
	int to_cachepool = 0;

	switch (cmd->command->command_enum) {
	case lvconvert_to_thinpool_CMD:
		to_thinpool = 1;
		break;
	case lvconvert_to_cachepool_CMD:
		to_cachepool = 1;
		break;
	default:
		log_error(INTERNAL_ERROR "Invalid lvconvert pool command");
		return 0;
	};

	if (cmd->position_argc > 1) {
		/* First pos arg is required LV, remaining are optional PVs. */
		if (!(use_pvh = create_pv_list(cmd->mem, lv->vg, cmd->position_argc - 1, cmd->position_argv + 1, 0)))
			return_ECMD_FAILED;
	} else
		use_pvh = &lv->vg->pvs;

	if (!_lvconvert_to_pool(cmd, lv, lv, to_thinpool, to_cachepool, use_pvh))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

/*
 * The LV position arg is used as thinpool/cachepool data LV.
 */

int lvconvert_to_pool_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	return process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, NULL, &_lvconvert_to_pool_single);
}

static int _lvconvert_to_cache_vol_single(struct cmd_context *cmd,
					 struct logical_volume *lv,
					 struct processing_handle *handle)
{
	struct volume_group *vg = lv->vg;
	struct logical_volume *cachepool_lv;
	const char *cachepool_name;

	if (!(cachepool_name = arg_str_value(cmd, cachepool_ARG, NULL)))
		goto_out;

	if (!validate_lvname_param(cmd, &vg->name, &cachepool_name))
		goto_out;

	if (!(cachepool_lv = find_lv(vg, cachepool_name))) {
		log_error("Cache pool %s not found.", cachepool_name);
		goto out;
	}

	/*
	 * If cachepool_lv is not yet a cache pool, convert it to one.
	 * If using an existing cache pool, wipe it.
	 */

	if (!lv_is_cache_pool(cachepool_lv)) {
		int lvt_enum = get_lvt_enum(cachepool_lv);
		struct lv_type *lvtype = get_lv_type(lvt_enum);

		if (lvt_enum != striped_LVT && lvt_enum != linear_LVT && lvt_enum != raid_LVT) {
			log_error("LV %s with type %s cannot be converted to a cache pool.",
				  display_lvname(cachepool_lv), lvtype ? lvtype->name : "unknown");
			goto out;
		}

		if (cachepool_lv == lv) {
			log_error("Use a different LV for cache pool LV and cache LV %s.",
				  display_lvname(cachepool_lv));
			goto out;
		}

		if (!_lvconvert_to_pool(cmd, cachepool_lv, lv, 0, 1, &vg->pvs)) {
			log_error("LV %s could not be converted to a cache pool.",
				  display_lvname(cachepool_lv));
			goto out;
		}

		if (!(cachepool_lv = find_lv(vg, cachepool_name))) {
			log_error("LV %s cannot be found.", display_lvname(cachepool_lv));
			goto out;
		}

		if (!lv_is_cache_pool(cachepool_lv)) {
			log_error("LV %s is not a cache pool.", display_lvname(cachepool_lv));
			goto out;
		}
	} else {
		if (!dm_list_empty(&cachepool_lv->segs_using_this_lv)) {
			log_error("Cache pool %s is already in use.", cachepool_name);
			goto out;
		}

		/* Note: requires rather deep know-how to skip zeroing */
		if (!arg_is_set(cmd, zero_ARG)) {
		       	if (!arg_is_set(cmd, yes_ARG) &&
			    yes_no_prompt("Do you want wipe existing metadata of cache pool %s? [y/n]: ",
					  display_lvname(cachepool_lv)) == 'n') {
				log_error("Conversion aborted.");
				log_error("To preserve cache metadata add option \"--zero n\".");
				log_warn("WARNING: Reusing mismatched cache pool metadata MAY DESTROY YOUR DATA!");
				goto out;
			}
			/* Wiping confirmed, go ahead */
			if (!wipe_cache_pool(cachepool_lv))
				goto_out;
		} else if (arg_int_value(cmd, zero_ARG, 0)) {
			if (!wipe_cache_pool(cachepool_lv))
				goto_out;
		} else {
			log_warn("WARNING: Reusing cache pool metadata %s for volume caching.",
				 display_lvname(cachepool_lv));
		}
	}

	/* When the lv arg is a thinpool, redirect command to data sub lv. */

	if (lv_is_thin_pool(lv)) {
		lv = seg_lv(first_seg(lv), 0);
		log_verbose("Redirecting operation to data sub LV %s.", display_lvname(lv));
	}

	/* Convert lv to cache vol using cachepool_lv. */

	if (!_lvconvert_to_cache_vol(cmd, lv, cachepool_lv))
		goto_out;

	return ECMD_PROCESSED;

 out:
	return ECMD_FAILED;
}

int lvconvert_to_cache_vol_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	return process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, NULL, &_lvconvert_to_cache_vol_single);
}

static int _lvconvert_to_thin_with_external_single(struct cmd_context *cmd,
					 struct logical_volume *lv,
					 struct processing_handle *handle)
{
	struct volume_group *vg = lv->vg;
	struct logical_volume *thinpool_lv;
	const char *thinpool_name;

	if (!(thinpool_name = arg_str_value(cmd, thinpool_ARG, NULL)))
		goto_out;

	if (!validate_lvname_param(cmd, &vg->name, &thinpool_name))
		goto_out;

	if (!(thinpool_lv = find_lv(vg, thinpool_name))) {
		log_error("Thin pool %s not found.", thinpool_name);
		goto out;
	}

	/* If thinpool_lv is not yet a thin pool, convert it to one. */

	if (!lv_is_thin_pool(thinpool_lv)) {
		int lvt_enum = get_lvt_enum(thinpool_lv);
		struct lv_type *lvtype = get_lv_type(lvt_enum);

		if (lvt_enum != striped_LVT && lvt_enum != linear_LVT && lvt_enum != raid_LVT) {
			log_error("LV %s with type %s cannot be converted to a thin pool.",
				  display_lvname(thinpool_lv), lvtype ? lvtype->name : "unknown");
			goto out;
		}

		if (thinpool_lv == lv) {
			log_error("Use a different LV for thin pool LV and thin LV %s.",
				  display_lvname(thinpool_lv));
			goto out;
		}

		if (!_lvconvert_to_pool(cmd, thinpool_lv, lv, 1, 0, &vg->pvs)) {
			log_error("LV %s could not be converted to a thin pool.",
				  display_lvname(thinpool_lv));
			goto out;
		}

		if (!(thinpool_lv = find_lv(vg, thinpool_name))) {
			log_error("LV %s cannot be found.", display_lvname(thinpool_lv));
			goto out;
		}

		if (!lv_is_thin_pool(thinpool_lv)) {
			log_error("LV %s is not a thin pool.", display_lvname(thinpool_lv));
			goto out;
		}
	}

	/* If lv is a cache volume, all data must be flushed. */

	if (lv_is_cache(lv)) {
		const struct lv_segment *pool_seg = first_seg(first_seg(lv)->pool_lv);
		int is_clean;

		if (pool_seg->cache_mode != CACHE_MODE_WRITETHROUGH) {
			log_error("Cannot convert cache volume %s with %s cache mode to external origin.",
				  display_lvname(lv), get_cache_mode_name(pool_seg));
			log_error("To proceed, run 'lvchange --cachemode writethrough %s'.",
				  display_lvname(lv));
			goto out;
		}

		if (!lv_cache_wait_for_clean(lv, &is_clean))
			goto_out;

		if (!is_clean) {
			log_error("Cache %s is not clean, refusing to convert to external origin.",
				  display_lvname(lv));
			goto out;
		}
	}

	/* Convert lv to thin with external origin using thinpool_lv. */

	if (!_lvconvert_to_thin_with_external(cmd, lv, thinpool_lv))
		goto_out;

	return ECMD_PROCESSED;

 out:
	return ECMD_FAILED;
}

int lvconvert_to_thin_with_external_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	return process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, NULL, &_lvconvert_to_thin_with_external_single);
}

static int _lvconvert_swap_pool_metadata_single(struct cmd_context *cmd,
					 struct logical_volume *lv,
					 struct processing_handle *handle)
{
	struct volume_group *vg = lv->vg;
	struct logical_volume *metadata_lv;
	const char *metadata_name;

	if (vg_is_shared(lv->vg)) {
		/* FIXME: need to swap locks betwen LVs? */
		log_error("Unable to swap pool metadata in VG with lock_type %s", lv->vg->lock_type);
		goto out;
	}

	if (!(metadata_name = arg_str_value(cmd, poolmetadata_ARG, NULL)))
		goto_out;

	if (!validate_lvname_param(cmd, &vg->name, &metadata_name))
		goto_out;

	if (!(metadata_lv = find_lv(vg, metadata_name))) {
		log_error("Metadata LV %s not found.", metadata_name);
		goto out;
	}

	if (metadata_lv == lv) {
		log_error("Can't use same LV for pool data and metadata LV %s.",
			  display_lvname(metadata_lv));
		goto out;
	}

	if (!_lvconvert_swap_pool_metadata(cmd, lv, metadata_lv))
		goto_out;

	return ECMD_PROCESSED;

 out:
	return ECMD_FAILED;
}

int lvconvert_swap_pool_metadata_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	return process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, NULL, &_lvconvert_swap_pool_metadata_single);
}

static int _lvconvert_to_pool_or_swap_metadata_single(struct cmd_context *cmd,
					 struct logical_volume *lv,
					 struct processing_handle *handle)
{
	struct dm_list *use_pvh = NULL;
	int to_thinpool = 0;
	int to_cachepool = 0;

	switch (cmd->command->command_enum) {
	case lvconvert_to_thinpool_or_swap_metadata_CMD:
		to_thinpool = 1;
		break;
	case lvconvert_to_cachepool_or_swap_metadata_CMD:
		to_cachepool = 1;
		break;
	default:
		log_error(INTERNAL_ERROR "Invalid lvconvert pool command");
		return 0;
	};

	if (lv_is_origin(lv)) {
		log_error("Cannot convert logical volume %s under snapshot.",
			  display_lvname(lv));
		return 0;
	};

	if (cmd->position_argc > 1) {
		/* First pos arg is required LV, remaining are optional PVs. */
		if (!(use_pvh = create_pv_list(cmd->mem, lv->vg, cmd->position_argc - 1, cmd->position_argv + 1, 0)))
			return_ECMD_FAILED;
	} else
		use_pvh = &lv->vg->pvs;

	/*
	 * We can finally determine if this command is supposed to create
	 * a pool or swap the metadata in an existing pool.
	 *
	 * This allows the ambiguous command:
	 * 'lvconvert --thinpool LV1 --poolmetadata LV2' to mean either:
	 * 1. convert LV2 to a pool using the specified meta LV2
	 * 2. swap the meta lv in LV1 with LV2
	 *
	 * In case 2, the poolmetadata option is required, but in case 1
	 * it is optional.  So, the command def is not able to validate
	 * the required/optional option, and we have to check here
	 * for missing poolmetadata in case 2.
	 */
	if (lv_is_pool(lv)) {
		if (!arg_is_set(cmd, poolmetadata_ARG)) {
			log_error("The --poolmetadata option is required to swap metadata.");
			return ECMD_FAILED;
		}
		return _lvconvert_swap_pool_metadata_single(cmd, lv, handle);
	}

	if (!_lvconvert_to_pool(cmd, lv, lv, to_thinpool, to_cachepool, use_pvh))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

/*
 * In the command variants with no position LV arg, the LV arg is taken from
 * the --thinpool/--cachepool arg, and the position args are modified to match
 * the standard command form.
 */

int lvconvert_to_pool_or_swap_metadata_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	char *pool_data_name;
	int i, p;

	switch (cmd->command->command_enum) {
	case lvconvert_to_thinpool_or_swap_metadata_CMD:
		pool_data_name = (char *)arg_str_value(cmd, thinpool_ARG, NULL);
		break;
	case lvconvert_to_cachepool_or_swap_metadata_CMD:
		pool_data_name = (char *)arg_str_value(cmd, cachepool_ARG, NULL);
		break;
	default:
		log_error(INTERNAL_ERROR "Unknown pool conversion.");
		return 0;
	};

	/* Make the LV the first position arg. */

	p = cmd->position_argc;
	for (i = 0; i < cmd->position_argc; i++)
		cmd->position_argv[p] = cmd->position_argv[p-1];

	cmd->position_argv[0] = pool_data_name;
	cmd->position_argc++;

	return process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, NULL, &_lvconvert_to_pool_or_swap_metadata_single);
}

static int _lvconvert_merge_thin_single(struct cmd_context *cmd,
					 struct logical_volume *lv,
					 struct processing_handle *handle)
{
	if (!_lvconvert_merge_thin_snapshot(cmd, lv))
		return ECMD_FAILED;

	return ECMD_PROCESSED;
}

int lvconvert_merge_thin_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	return process_each_lv(cmd, cmd->position_argc, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, NULL, &_lvconvert_merge_thin_single);
}

static int _lvconvert_split_cachepool_single(struct cmd_context *cmd,
					 struct logical_volume *lv,
					 struct processing_handle *handle)
{
	struct logical_volume *cache_lv = NULL;
	struct logical_volume *cachepool_lv = NULL;
	struct lv_segment *seg;
	int ret;

	if (lv_is_cache(lv)) {
		cache_lv = lv;
		cachepool_lv = first_seg(cache_lv)->pool_lv;

	} else if (lv_is_cache_pool(lv)) {
		cachepool_lv = lv;

		if ((dm_list_size(&cachepool_lv->segs_using_this_lv) == 1) &&
		    (seg = get_only_segment_using_this_lv(cachepool_lv)) &&
		    seg_is_cache(seg))
			cache_lv = seg->lv;

	} else if (lv_is_thin_pool(lv)) {
		cache_lv = seg_lv(first_seg(lv), 0); /* cached _tdata */
		cachepool_lv = first_seg(cache_lv)->pool_lv;
	}

	if (!cache_lv) {
		log_error("Cannot find cache LV from %s.", display_lvname(lv));
		return ECMD_FAILED;
	}

	if (!cachepool_lv) {
		log_error("Cannot find cache pool LV from %s.", display_lvname(lv));
		return ECMD_FAILED;
	}

	/* If LV is inactive here, ensure it's not active elsewhere. */
	if (!lockd_lv(cmd, cache_lv, "ex", 0))
		return_0;

	switch (cmd->command->command_enum) {
	case lvconvert_split_and_keep_cachepool_CMD:
		ret = _lvconvert_split_and_keep_cachepool(cmd, cache_lv, cachepool_lv);
		break;

	case lvconvert_split_and_remove_cachepool_CMD:
		ret = _lvconvert_split_and_remove_cachepool(cmd, cache_lv, cachepool_lv);
		break;
	default:
		log_error(INTERNAL_ERROR "Unknown cache pool split.");
		ret = 0;
	}

	if (!ret)
		return ECMD_FAILED;

	return ECMD_PROCESSED;
}

int lvconvert_split_cachepool_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	if (cmd->command->command_enum == lvconvert_split_and_remove_cachepool_CMD) {
		cmd->handles_missing_pvs = 1;
		cmd->partial_activation = 1;
	}

	return process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, NULL, &_lvconvert_split_cachepool_single);
}

static int _lvconvert_raid_types_single(struct cmd_context *cmd, struct logical_volume *lv,
			     struct processing_handle *handle)
{
	struct lvconvert_params *lp = (struct lvconvert_params *) handle->custom_handle;
	struct dm_list *use_pvh;
	struct convert_poll_id_list *idl;
	struct lvinfo info;
	int ret;

	if (cmd->position_argc > 1) {
		/* First pos arg is required LV, remaining are optional PVs. */
		if (!(use_pvh = create_pv_list(cmd->mem, lv->vg, cmd->position_argc - 1, cmd->position_argv + 1, 0)))
			return_ECMD_FAILED;
		lp->pv_count = cmd->position_argc - 1;
	} else
		use_pvh = &lv->vg->pvs;

	lp->pvh = use_pvh;

	lp->lv_to_poll = lv;

	ret = _lvconvert_raid_types(cmd, lv, lp);

	if (ret != ECMD_PROCESSED)
		return_ECMD_FAILED;

	if (lp->need_polling) {
		/* _lvconvert() call may alter the reference in lp->lv_to_poll */
		if (!lv_info(cmd, lp->lv_to_poll, 0, &info, 0, 0) || !info.exists)
			log_print_unless_silent("Conversion starts after activation.");
		else {
			if (!(idl = _convert_poll_id_list_create(cmd, lp->lv_to_poll)))
				return_ECMD_FAILED;
			dm_list_add(&lp->idls, &idl->list);
		}
	}

	return ECMD_PROCESSED;
}

static int _lvconvert_raid_types_check(struct cmd_context *cmd, struct logical_volume *lv,
			struct processing_handle *handle,
			int lv_is_named_arg)
{
	int lvt_enum = get_lvt_enum(lv);
	struct lv_type *lvtype = get_lv_type(lvt_enum);

	if (!lv_is_visible(lv)) {
		if (!lv_is_cache_pool_metadata(lv) &&
		    !lv_is_cache_pool_data(lv) &&
		    !lv_is_thin_pool_metadata(lv) &&
		    !lv_is_thin_pool_data(lv) &&
		    !lv_is_used_cache_pool(lv) &&
		    !lv_is_mirrored(lv) &&
		    !lv_is_raid(lv))
			goto fail_hidden;
	}

	/*
	 * FIXME: this validation could be done by command defs.
	 *
	 * Outside the standard linear/striped/mirror/raid LV
	 * types, cache is the only special LV type that is handled
	 * (the command is redirected to origin).
	 */
	switch (lvt_enum) {
	case thin_LVT:
	case thinpool_LVT:
	case cachepool_LVT:
	case snapshot_LVT:
		log_error("Operation not permitted on LV %s type %s.",
			  display_lvname(lv), lvtype ? lvtype->name : "unknown");
		return 0;
	}

	return 1;

 fail_hidden:
	log_error("Operation not permitted on hidden LV %s.", display_lvname(lv));
	return 0;
}

int lvconvert_raid_types_cmd(struct cmd_context * cmd, int argc, char **argv)
{
	int poll_ret, ret;
	int saved_ignore_suspended_devices;
	struct processing_handle *handle;
	struct convert_poll_id_list *idl;
	struct lvconvert_params lp = {
		.conv_type = CONV_OTHER,
		.target_attr = ~0,
		.idls = DM_LIST_HEAD_INIT(lp.idls),
	};

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &lp;

	if (!_read_params(cmd, &lp)) {
		ret = EINVALID_CMD_LINE;
		goto_out;
	}

	saved_ignore_suspended_devices = ignore_suspended_devices();

	ret = process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			      handle, &_lvconvert_raid_types_check, &_lvconvert_raid_types_single);

	init_ignore_suspended_devices(saved_ignore_suspended_devices);

	dm_list_iterate_items(idl, &lp.idls) {
		poll_ret = _lvconvert_poll_by_id(cmd, idl->id,
						 lp.wait_completion ? 0 : 1U,
						 idl->is_merging_origin,
						 idl->is_merging_origin_thin);
		if (poll_ret > ret)
			ret = poll_ret;
	}

out:
	destroy_processing_handle(cmd, handle);

	return ret;
}

/*
 * change mirror log
 */

static int _lvconvert_visible_check(struct cmd_context *cmd, struct logical_volume *lv,
			struct processing_handle *handle,
			int lv_is_named_arg)
{
	if (!lv_is_visible(lv)) {
		log_error("Operation not permitted on hidden LV %s.", display_lvname(lv));
		return 0;
	}

	return 1;
}

static int _lvconvert_change_mirrorlog_single(struct cmd_context *cmd, struct logical_volume *lv,
			     struct processing_handle *handle)
{
	struct lvconvert_params *lp = (struct lvconvert_params *) handle->custom_handle;
	struct dm_list *use_pvh;

	if (cmd->position_argc > 1) {
		/* First pos arg is required LV, remaining are optional PVs. */
		if (!(use_pvh = create_pv_list(cmd->mem, lv->vg, cmd->position_argc - 1, cmd->position_argv + 1, 0)))
			return_ECMD_FAILED;
		lp->pv_count = cmd->position_argc - 1;
	} else
		use_pvh = &lv->vg->pvs;

	lp->pvh = use_pvh;

	/* FIXME: extract the mirrorlog functionality out of _lvconvert_raid_types()? */
	return _lvconvert_raid_types(cmd, lv, lp);
}

int lvconvert_change_mirrorlog_cmd(struct cmd_context * cmd, int argc, char **argv)
{
	struct processing_handle *handle;
	struct lvconvert_params lp = {
		.conv_type = CONV_OTHER,
		.target_attr = ~0,
		.idls = DM_LIST_HEAD_INIT(lp.idls),
	};
	int ret;

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &lp;

	/* FIXME: extract the relevant bits of read_params and put here. */
	if (!_read_params(cmd, &lp)) {
		ret = EINVALID_CMD_LINE;
		goto_out;
	}

	ret = process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			      handle, &_lvconvert_visible_check, &_lvconvert_change_mirrorlog_single);

out:
	destroy_processing_handle(cmd, handle);

	return ret;
}

static int _lvconvert_change_region_size_single(struct cmd_context *cmd, struct logical_volume *lv,
			     struct processing_handle *handle)
{
	if (!lv_raid_change_region_size(lv, arg_is_set(cmd, yes_ARG), arg_count(cmd, force_ARG),
			                arg_int_value(cmd, regionsize_ARG, 0)))
		return ECMD_FAILED;
	return ECMD_PROCESSED;
}

int lvconvert_change_region_size_cmd(struct cmd_context * cmd, int argc, char **argv)
{
	return process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			      NULL, &_lvconvert_visible_check, &_lvconvert_change_region_size_single);
}

/*
 * split mirror images
 */

static int _lvconvert_split_mirror_images_single(struct cmd_context *cmd, struct logical_volume *lv,
			     struct processing_handle *handle)
{
	struct lvconvert_params *lp = (struct lvconvert_params *) handle->custom_handle;
	struct dm_list *use_pvh;

	if (cmd->position_argc > 1) {
		/* First pos arg is required LV, remaining are optional PVs. */
		if (!(use_pvh = create_pv_list(cmd->mem, lv->vg, cmd->position_argc - 1, cmd->position_argv + 1, 0)))
			return_ECMD_FAILED;
		lp->pv_count = cmd->position_argc - 1;
	} else
		use_pvh = &lv->vg->pvs;

	lp->pvh = use_pvh;

	/* FIXME: extract the split functionality out of _lvconvert_raid_types()? */
	return _lvconvert_raid_types(cmd, lv, lp);
}

int lvconvert_split_mirror_images_cmd(struct cmd_context * cmd, int argc, char **argv)
{
	struct processing_handle *handle;
	struct lvconvert_params lp = {
		.conv_type = CONV_OTHER,
		.target_attr = ~0,
		.idls = DM_LIST_HEAD_INIT(lp.idls),
	};
	int ret;

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &lp;

	/* FIXME: extract the relevant bits of read_params and put here. */
	if (!_read_params(cmd, &lp)) {
		ret = EINVALID_CMD_LINE;
		goto_out;
	}

	/* FIXME: are there any hidden LVs that should be disallowed? */

	ret = process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			      handle, NULL, &_lvconvert_split_mirror_images_single);

out:
	destroy_processing_handle(cmd, handle);

	return ret;
}

/*
 * merge mirror images
 *
 * Called from both lvconvert --mergemirrors and lvconvert --merge.
 */

static int _lvconvert_merge_mirror_images_single(struct cmd_context *cmd,
                                          struct logical_volume *lv,
                                          struct processing_handle *handle)
{
	if (!lv_raid_merge(lv))
		return ECMD_FAILED;

	return ECMD_PROCESSED;
}

int lvconvert_merge_mirror_images_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	/* arg can be a VG name, which is the standard option usage */
	cmd->cname->flags &= ~GET_VGNAME_FROM_OPTIONS;

	return process_each_lv(cmd, cmd->position_argc, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, &_lvconvert_visible_check, &_lvconvert_merge_mirror_images_single);
}

static int _lvconvert_merge_generic_single(struct cmd_context *cmd,
					 struct logical_volume *lv,
					 struct processing_handle *handle)
{
	int ret;

	if (lv_is_cow(lv))
		ret = _lvconvert_merge_snapshot_single(cmd, lv, handle);

	else if (lv_is_thin_volume(lv))
		ret = _lvconvert_merge_thin_single(cmd, lv, handle);

	else
		ret = _lvconvert_merge_mirror_images_single(cmd, lv, handle);

	return ret;
}

int lvconvert_merge_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	struct processing_handle *handle;
	struct lvconvert_result lr = { 0 };
	struct convert_poll_id_list *idl;
	int ret, poll_ret;

	dm_list_init(&lr.poll_idls);

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &lr;

	cmd->cname->flags &= ~GET_VGNAME_FROM_OPTIONS;

	ret = process_each_lv(cmd, cmd->position_argc, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			      handle, NULL, &_lvconvert_merge_generic_single);

	/* polling is only used by merge_snapshot */
	if (lr.need_polling) {
		dm_list_iterate_items(idl, &lr.poll_idls) {
			poll_ret = _lvconvert_poll_by_id(cmd, idl->id,
						arg_is_set(cmd, background_ARG), 1, 0);
			if (poll_ret > ret)
				ret = poll_ret;
		}
	}

	destroy_processing_handle(cmd, handle);

	return ret;
}

static int _lvconvert_to_vdopool_single(struct cmd_context *cmd,
					struct logical_volume *lv,
					struct processing_handle *handle)
{
	const char *vg_name = NULL;
	struct volume_group *vg = lv->vg;
	struct logical_volume *vdo_lv;
	struct dm_vdo_target_params vdo_params; /* vdo */
	struct lvcreate_params lvc = {
		.activate = CHANGE_AEY,
		.alloc = ALLOC_INHERIT,
		.major = -1,
		.minor = -1,
		.suppress_zero_warn = 1, /* Suppress warning for this VDO */
		.permission = LVM_READ | LVM_WRITE,
		.pool_name = lv->name,
		.pvh = &vg->pvs,
		.read_ahead = DM_READ_AHEAD_AUTO,
		.stripes = 1,
		.lv_name = arg_str_value(cmd, name_ARG, NULL),
	};

	if (lvc.lv_name &&
	    !validate_restricted_lvname_param(cmd, &vg_name, &lvc.lv_name))
		return_0;

	lvc.virtual_extents = extents_from_size(cmd,
						arg_uint_value(cmd, virtualsize_ARG, 0),
						vg->extent_size);

	if (!(lvc.segtype = get_segtype_from_string(cmd, SEG_TYPE_NAME_VDO)))
		return_0;

	if (vg_is_shared(vg)) {
		/* FIXME: need to swap locks betwen LVs? */
		log_error("Unable to convert VDO pool in VG with lock_type %s", vg->lock_type);
		goto out;
	}

	if (!fill_vdo_target_params(cmd, &vdo_params, NULL))
		goto_out;

	if (arg_is_set(cmd, compression_ARG))
		vdo_params.use_compression =
			arg_int_value(cmd, compression_ARG, 0);

	if (arg_is_set(cmd, deduplication_ARG))
		vdo_params.use_deduplication =
			arg_int_value(cmd, deduplication_ARG, 0);

	if (!activate_lv(cmd, lv)) {
		log_error("Cannot activate %s.", display_lvname(lv));
		goto out;
	}

	log_warn("WARNING: Converting logical volume %s to VDO pool volume.",
		 lv->name);
	log_warn("THIS WILL DESTROY CONTENT OF LOGICAL VOLUME (filesystem etc.)");

	if (!arg_count(cmd, yes_ARG) &&
	    yes_no_prompt("Do you really want to convert %s? [y/n]: ",
			  lv->name) == 'n') {
		log_error("Conversion aborted.");
		goto out;
	}

	if (!wipe_lv(lv, (struct wipe_params) { .do_zero = 1, .do_wipe_signatures = 1 })) {
		log_error("Aborting. Failed to wipe VDO data store.");
		goto out;
	}

	if (!archive(vg))
		goto_out;

	if (!convert_vdo_pool_lv(lv, &vdo_params, &lvc.virtual_extents))
		goto_out;

	dm_list_init(&lvc.tags);

	if (!(vdo_lv = lv_create_single(vg, &lvc)))
		goto_out; /* FIXME: hmmm what to do now */

	log_print_unless_silent("Converted %s to VDO pool volume and created virtual %s VDO volume.",
				display_lvname(lv), display_lvname(vdo_lv));

	return ECMD_PROCESSED;

 out:
	return ECMD_FAILED;
}

int lvconvert_to_vdopool_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	return process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, NULL, &_lvconvert_to_vdopool_single);
}

int lvconvert_to_vdopool_param_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	/* Make the LV the first position arg. */
	int i, p = cmd->position_argc;

	for (i = 0; i < cmd->position_argc; i++)
		cmd->position_argv[p] = cmd->position_argv[p-1];

	cmd->position_argv[0] = (char *)arg_str_value(cmd, vdopool_ARG, NULL);
	cmd->position_argc++;

	return process_each_lv(cmd, 1, cmd->position_argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, NULL, &_lvconvert_to_vdopool_single);
}

/*
 * All lvconvert command defs have their own function,
 * so the generic function name is unused.
 */

int lvconvert(struct cmd_context *cmd, int argc, char **argv)
{
	log_error(INTERNAL_ERROR "Missing function for command definition %d:%s.",
		  cmd->command->command_index, cmd->command->command_id);
	return ECMD_FAILED;
}
