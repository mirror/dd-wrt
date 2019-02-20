/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2014 Red Hat, Inc. All rights reserved.
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

#include <fcntl.h>

struct lvcreate_cmdline_params {
	percent_type_t percent;
	uint64_t size;
	uint64_t virtual_size; /* snapshot, thin */
	char **pvs;
	uint32_t pv_count;
};

struct processing_params {
	struct lvcreate_params *lp;
	struct lvcreate_cmdline_params *lcp;
};

static int _set_vg_name(struct lvcreate_params *lp, const char *vg_name)
{
	/* Can't do anything */
	if (!vg_name)
		return 1;

	/* If VG name already known, ensure this 2nd copy is identical */
	if (lp->vg_name && strcmp(lp->vg_name, vg_name)) {
		log_error("Inconsistent volume group names "
			  "given: \"%s\" and \"%s\"",
			  lp->vg_name, vg_name);
		return 0;
	}
	lp->vg_name = vg_name;

	return 1;
}

static int _lvcreate_name_params(struct cmd_context *cmd,
				 int *pargc, char ***pargv,
				 struct lvcreate_params *lp)
{
	int argc = *pargc;
	char **argv = *pargv;
	const char *vg_name;

	lp->lv_name = arg_str_value(cmd, name_ARG, NULL);
	if (!validate_restricted_lvname_param(cmd, &lp->vg_name, &lp->lv_name))
		return_0;

	lp->pool_name = arg_str_value(cmd, thinpool_ARG, NULL)
		? : arg_str_value(cmd, vdopool_ARG, NULL)
		? : arg_str_value(cmd, cachepool_ARG, NULL);
	if (!validate_lvname_param(cmd, &lp->vg_name, &lp->pool_name))
		return_0;

	if (seg_is_cache(lp)) {
		/*
		 * 2 ways of cache usage for lvcreate -H -l1 vg/lv
		 *
		 * vg/lv is existing cache pool:
		 *       cached LV is created using this cache pool
		 * vg/lv is not cache pool so it is cache origin
		 *       origin is cached with created cache pool
		 *
		 * We are looking for the vgname or cache pool or cache origin LV.
		 *
		 * lv name is stored in origin_name and pool_name and
		 * later with opened VG it's decided what should happen.
		 */
		if (!argc) {
			if (!lp->pool_name) {
				log_error("Please specify a logical volume to act as the cache pool or origin.");
				return 0;
			}
		} else {
			vg_name = skip_dev_dir(cmd, argv[0], NULL);
			if (!strchr(vg_name, '/')) {
				/* Lucky part - only vgname is here */
				if (!_set_vg_name(lp, vg_name))
					return_0;
			} else {
				/* Assume it's cache origin for now */
				lp->origin_name = vg_name;
				if (!validate_lvname_param(cmd, &lp->vg_name, &lp->origin_name))
					return_0;

				if (lp->pool_name) {
					if (strcmp(lp->pool_name, lp->origin_name)) {
						log_error("Unsupported syntax, cannot use cache origin %s and --cachepool %s.",
							  lp->origin_name, lp->pool_name);
						return 0;
					}
					lp->origin_name = NULL;
				} else {
					/*
					 * Gambling here, could be cache pool or cache origin,
					 * detection is possible after openning vg,
					 * yet we need to parse pool args
					 */
					lp->pool_name = lp->origin_name;
					lp->create_pool = 1;
				}
			}
			(*pargv)++, (*pargc)--;
		}

		if (!lp->vg_name &&
		    !_set_vg_name(lp, extract_vgname(cmd, NULL)))
			return_0;

		if (!lp->vg_name) {
			log_error("The cache pool or cache origin name should "
				  "include the volume group.");
			return 0;
		}

		if (!lp->pool_name) {
			log_error("Creation of cached volume and cache pool "
				  "in one command is not yet supported.");
			return 0;
		}
	} else if (lp->snapshot && !arg_is_set(cmd, virtualsize_ARG)) {
		/* argv[0] might be [vg/]origin */
		if (!argc) {
			log_error("Please specify a logical volume to act as "
				  "the snapshot origin.");
			return 0;
		}

		lp->origin_name = argv[0];
		if (!validate_lvname_param(cmd, &lp->vg_name, &lp->origin_name))
			return_0;

		if (!lp->vg_name &&
		    !_set_vg_name(lp, extract_vgname(cmd, NULL)))
			return_0;

		if (!lp->vg_name) {
			log_error("The origin name should include the "
				  "volume group.");
			return 0;
		}

		(*pargv)++, (*pargc)--;
	} else if ((seg_is_pool(lp) || seg_is_thin(lp) || seg_is_vdo(lp)) && argc) {
		/* argv[0] might be [/dev.../]vg or [/dev../]vg/pool */

		vg_name = skip_dev_dir(cmd, argv[0], NULL);
		if (!strchr(vg_name, '/')) {
			if (lp->snapshot && arg_is_set(cmd, virtualsize_ARG))
				lp->snapshot = 0 ; /* Sparse volume via thin-pool */
			if (!_set_vg_name(lp, vg_name))
				return_0;
		} else {
			if (!validate_lvname_param(cmd, &lp->vg_name, &vg_name))
				return_0;

			if (lp->pool_name &&
			    (strcmp(vg_name, lp->pool_name) != 0)) {
				log_error("Ambiguous %s name specified, %s and %s.",
					  lp->segtype->name, vg_name, lp->pool_name);
				return 0;
			}
			lp->pool_name = vg_name;

			if (!lp->vg_name &&
			    !_set_vg_name(lp, extract_vgname(cmd, NULL)))
				return_0;

			if (!lp->vg_name) {
				log_error("The %s name should include the "
					  "volume group.", lp->segtype->name);
				return 0;
			}
		}
		(*pargv)++, (*pargc)--;
	} else {
		/*
		 * If VG not on command line, try environment default.
		 */
		if (!argc) {
			if (!lp->vg_name && !(lp->vg_name = extract_vgname(cmd, NULL))) {
				log_error("Please provide a volume group name");
				return 0;
			}
		} else {
			vg_name = skip_dev_dir(cmd, argv[0], NULL);
			if (strchr(vg_name, '/')) {
				log_error("Volume group name expected "
					  "(no slash)");
				return 0;
			}

			if (!_set_vg_name(lp, vg_name))
				return_0;

			(*pargv)++, (*pargc)--;
		}
	}

	/* support --name & --type {thin|cache}-pool */
	if (seg_is_pool(lp) && lp->lv_name) {
		if (lp->pool_name && (strcmp(lp->lv_name, lp->pool_name) != 0)) {
			log_error("Ambiguous %s name specified, %s and %s.",
				  lp->segtype->name, lp->lv_name, lp->pool_name);
			return 0;
		}
		lp->pool_name = lp->lv_name;
		lp->lv_name = NULL;
	}

	if (lp->pool_name && lp->lv_name && !strcmp(lp->pool_name, lp->lv_name)) {
		log_error("Logical volume name %s and pool name must be different.",
			  lp->lv_name);
		return 0;
	}

	if (!validate_name(lp->vg_name)) {
		log_error("Volume group name %s has invalid characters",
			  lp->vg_name);
		return 0;
	}

	return 1;
}

/*
 * Update extents parameters based on other parameters which affect the size
 * calculation.
 * NOTE: We must do this here because of the dm_percent_t typedef and because we
 * need the vg.
 */
static int _update_extents_params(struct volume_group *vg,
				  struct lvcreate_params *lp,
				  struct lvcreate_cmdline_params *lcp)
{
	uint32_t pv_extent_count;
	struct logical_volume *origin_lv = NULL;
	uint32_t size_rest;
	uint32_t stripesize_extents;
	uint32_t extents;
	uint32_t base_calc_extents;

	if (lcp->size &&
	    !(lp->extents = extents_from_size(vg->cmd, lcp->size,
					       vg->extent_size)))
		return_0;

	if (lcp->virtual_size &&
	    !(lp->virtual_extents = extents_from_size(vg->cmd, lcp->virtual_size,
						      vg->extent_size)))
		return_0;

	/*
	 * Create the pv list before we parse lcp->percent - might be
	 * PERCENT_PVSs
	 */
	if (lcp->pv_count) {
		if (!(lp->pvh = create_pv_list(vg->cmd->mem, vg,
					       lcp->pv_count, lcp->pvs, 1)))
			return_0;
	} else
		lp->pvh = &vg->pvs;

	switch (lcp->percent) {
		case PERCENT_VG:
			extents = percent_of_extents(lp->extents, base_calc_extents = vg->extent_count, 0);
			break;
		case PERCENT_FREE:
			extents = percent_of_extents(lp->extents, base_calc_extents = vg->free_count, 0);
			break;
		case PERCENT_PVS:
			if (lcp->pv_count) {
				pv_extent_count = pv_list_extents_free(lp->pvh);
				extents = percent_of_extents(lp->extents, base_calc_extents = pv_extent_count, 0);
			} else
				extents = percent_of_extents(lp->extents, base_calc_extents = vg->extent_count, 0);
			break;
		case PERCENT_LV:
			log_error("Please express size as %%FREE%s, %%PVS or %%VG.",
				  (lp->snapshot) ? ", %ORIGIN" : "");
			return 0;
		case PERCENT_ORIGIN:
			if (lp->snapshot && lp->origin_name &&
			    !(origin_lv = find_lv(vg, lp->origin_name))) {
				log_error("Couldn't find origin volume '%s'.",
					  lp->origin_name);
				return 0;
			}
			if (!origin_lv) {
				log_error(INTERNAL_ERROR "Couldn't find origin volume.");
				return 0;
			}
			/* Add whole metadata size estimation */
			extents = cow_max_extents(origin_lv, lp->chunk_size) - origin_lv->le_count +
				percent_of_extents(lp->extents, base_calc_extents = origin_lv->le_count, 1);
			break;
		case PERCENT_NONE:
			extents = lp->extents;
			break;
		default:
			log_error(INTERNAL_ERROR "Unsupported percent type %u.", lcp->percent);
			return 0;
	}

	if (lcp->percent != PERCENT_NONE) {
		/* FIXME Don't do the adjustment for parallel allocation with PERCENT_ORIGIN! */
		lp->approx_alloc = 1;
		if (!extents) {
			log_error("Calculated size of logical volume is 0 extents. Needs to be larger.");
			return 0;
		}

		/* For mirrors and raid with percentages based on physical extents, convert the total number of PEs 
		 * into the number of logical extents per image (minimum 1) */
		/* FIXME Handle all the supported raid layouts here based on already-known segtype. */
		if ((lcp->percent != PERCENT_ORIGIN) && lp->mirrors) {
			extents /= lp->mirrors;
			if (!extents)
				extents = 1;
		}

		log_verbose("Converted %" PRIu32 "%% of %s (%" PRIu32 ") extents into %" PRIu32 " (with mimages %" PRIu32 " and stripes %" PRIu32
			    " for segtype %s).", lp->extents, get_percent_string(lcp->percent), base_calc_extents,
			    extents, lp->mirrors, lp->stripes, lp->segtype->name);

		lp->extents = extents;
	}

	if (lp->snapshot && lp->origin_name && lp->extents) {
		if (!lp->chunk_size) {
			log_error(INTERNAL_ERROR "Missing snapshot chunk size.");
			return 0;
		}

		if (!origin_lv && !(origin_lv = find_lv(vg, lp->origin_name))) {
			log_error("Couldn't find origin volume '%s'.",
				  lp->origin_name);
			return 0;
		}

		extents = cow_max_extents(origin_lv, lp->chunk_size);

		if (extents < lp->extents) {
			log_print_unless_silent("Reducing COW size %s down to maximum usable size %s.",
						display_size(vg->cmd, (uint64_t) vg->extent_size * lp->extents),
						display_size(vg->cmd, (uint64_t) vg->extent_size * extents));
			lp->extents = extents;
		}
	}

	if (!(stripesize_extents = lp->stripe_size / vg->extent_size))
		stripesize_extents = 1;

	if ((lcp->percent != PERCENT_NONE) && lp->stripes &&
	    (size_rest = lp->extents % (lp->stripes * stripesize_extents)) &&
	    (vg->free_count < lp->extents - size_rest + (lp->stripes * stripesize_extents))) {
		log_print_unless_silent("Rounding size (%d extents) down to stripe boundary "
					"size (%d extents)", lp->extents,
					lp->extents - size_rest);
		lp->extents = lp->extents - size_rest;
	}

	if (lp->create_pool) {
		if (lp->pool_metadata_size &&
		    !(lp->pool_metadata_extents =
		      extents_from_size(vg->cmd, lp->pool_metadata_size, vg->extent_size)))
			return_0;

		if (segtype_is_thin_pool(lp->segtype) || segtype_is_thin(lp->segtype)) {
			if (!update_thin_pool_params(vg->cmd, vg->profile, vg->extent_size,
						     lp->segtype, lp->target_attr,
						     lp->extents,
						     &lp->pool_metadata_extents,
						     &lp->thin_chunk_size_calc_policy,
						     &lp->chunk_size,
						     &lp->discards,
						     &lp->zero_new_blocks))
				return_0;
		} else if (!update_cache_pool_params(vg->cmd, vg->profile, vg->extent_size,
						     lp->segtype, lp->target_attr,
						     lp->extents,
						     &lp->pool_metadata_extents,
						     &lp->thin_chunk_size_calc_policy,
						     &lp->chunk_size))
			return_0;

		if (lcp->percent == PERCENT_FREE || lcp->percent == PERCENT_PVS) {
			if (lp->extents <= (2 * lp->pool_metadata_extents)) {
				log_error("Not enough space for thin pool creation.");
				return 0;
			}
			/* FIXME: persistent hidden space in VG wanted */
			lp->extents -= (2 * lp->pool_metadata_extents);
		}
	}

	if ((lcp->percent != PERCENT_NONE) && !lp->extents) {
		log_error("Adjusted size of logical volume is 0 extents. Needs to be larger.");
		return 0;
	}

	return 1;
}

/*
 * Validate various common size arguments
 *
 * Note: at this place all volume types needs to be already
 *       identified, do not change them here.
 */
static int _read_size_params(struct cmd_context *cmd,
			     struct lvcreate_params *lp,
			     struct lvcreate_cmdline_params *lcp)
{
	if (arg_from_list_is_negative(cmd, "may not be negative",
				      chunksize_ARG, extents_ARG,
				      mirrors_ARG,
				      maxrecoveryrate_ARG,
				      minrecoveryrate_ARG,
				      regionsize_ARG,
				      size_ARG,
				      stripes_ARG, stripesize_ARG,
				      virtualsize_ARG,
				      -1))
		return_0;

	if (arg_from_list_is_zero(cmd, "may not be zero",
				  chunksize_ARG, extents_ARG,
				  regionsize_ARG,
				  size_ARG,
				  stripes_ARG, stripesize_ARG,
				  virtualsize_ARG,
				  -1))
		return_0;

	lcp->virtual_size = arg_uint64_value(cmd, virtualsize_ARG, UINT64_C(0));

	if (arg_is_set(cmd, extents_ARG)) {
		if (arg_is_set(cmd, size_ARG)) {
			log_error("Please specify either size or extents (not both).");
			return 0;
		}
		lp->extents = arg_uint_value(cmd, extents_ARG, 0);
		lcp->percent = arg_percent_value(cmd, extents_ARG, PERCENT_NONE);
	} else if (arg_is_set(cmd, size_ARG)) {
		lcp->size = arg_uint64_value(cmd, size_ARG, UINT64_C(0));
		lcp->percent = PERCENT_NONE;
	} else if (!lp->snapshot && !seg_is_thin_volume(lp)) {
		log_error("Please specify either size or extents.");
		return 0;
	}

	return 1;
}

/*
 * Read parameters related to mirrors
 */
static int _read_mirror_params(struct cmd_context *cmd,
			       struct lvcreate_params *lp)
{
	int corelog = arg_is_set(cmd, corelog_ARG);

	lp->log_count = arg_int_value(cmd, mirrorlog_ARG, corelog ? 0 : DEFAULT_MIRRORLOG);

	if (corelog && (lp->log_count != 0)) {
		log_error("Please use only one of --corelog or --mirrorlog.");
		return 0;
	}

	log_verbose("Setting logging type to %s", get_mirror_log_name(lp->log_count));

	return 1;
}

/*
 * Read parameters related to raids
 */
static int _read_raid_params(struct cmd_context *cmd,
			     struct lvcreate_params *lp)
{
	if (seg_is_mirrored(lp)) {
		if (segtype_is_raid10(lp->segtype)) {
			if (lp->stripes < 2) {
				/*
				 * RAID10 needs at least 4 stripes
				 */
				if (lp->stripes_supplied) {
					log_error("Minimum of 2 stripes required for %s.",
						  lp->segtype->name);
					return 0;
				}

				log_verbose("Using 2 stripes for %s.", lp->segtype->name);
				lp->stripes = 2;
			}

			/*
			 * FIXME: _check_raid_parameters devides by 2, which
			 *	  needs to change if we start supporting
			 *	  odd numbers of stripes with raid10
			 */
			lp->stripes *= 2;

		} else if (lp->stripes > 1) {
			/*
			 * RAID1 does not take a stripe arg
			 */
			log_error("Stripes argument cannot be used with segment type, %s",
				  lp->segtype->name);
			return 0;
		}

	} else if (seg_is_any_raid6(lp) && lp->stripes < 3) {
		if (lp->stripes_supplied) {
			log_error("Minimum of 3 stripes required for %s.", lp->segtype->name);
			return 0;
		}

		log_verbose("Using 3 stripes for %s.", lp->segtype->name);
		lp->stripes = 3;
	} else if (lp->stripes < 2) {
		if (lp->stripes_supplied) {
			log_error("Minimum of 2 stripes required for %s.", lp->segtype->name);
			return 0;
		}

		log_verbose("Using 2 stripes for %s.", lp->segtype->name);
		lp->stripes = 2;
	}

	if (seg_is_raid1(lp)) {
		if (lp->stripe_size) {
			log_error("Stripe size argument cannot be used with segment type, %s",
				  lp->segtype->name);
			return 0;
		}
	}

	if (arg_is_set(cmd, mirrors_ARG) && segtype_is_raid(lp->segtype) &&
	    !segtype_is_raid1(lp->segtype) && !segtype_is_raid10(lp->segtype)) {
		log_error("Mirror argument cannot be used with segment type, %s",
			  lp->segtype->name);
		return 0;
	}

	if (seg_is_any_raid0(lp))
		lp->region_size = 0;
	else {
		/* Rates are recorded in kiB/sec/disk, not sectors/sec/disk */
		lp->min_recovery_rate = arg_uint_value(cmd, minrecoveryrate_ARG, 0) / 2;
		lp->max_recovery_rate = arg_uint_value(cmd, maxrecoveryrate_ARG, 0) / 2;

		if (lp->min_recovery_rate > lp->max_recovery_rate) {
			log_error("Minimum recovery rate cannot be higher than maximum.");
			return 0;
		}
	}

	return 1;
}

/*
 * Read parameters related to mirrors and raids
 */
static int _read_mirror_and_raid_params(struct cmd_context *cmd,
					struct lvcreate_params *lp)
{
	unsigned max_images;

	if (seg_is_raid(lp)) {
		if (seg_is_raid1(lp))
			max_images = DEFAULT_RAID1_MAX_IMAGES;
		else {
			max_images = DEFAULT_RAID_MAX_IMAGES;
			if (seg_is_raid4(lp) ||
			    seg_is_any_raid5(lp))
				max_images--;
			else if (seg_is_any_raid6(lp))
				max_images -= 2;
		}
	} else if (seg_is_mirrored(lp))
		max_images = DEFAULT_MIRROR_MAX_IMAGES;
	else
		max_images = MAX_STRIPES;

	/* Common mirror and raid params */
	if (arg_is_set(cmd, mirrors_ARG)) {
		lp->mirrors = arg_uint_value(cmd, mirrors_ARG, 0) + 1;

		if ((lp->mirrors > 2) && segtype_is_raid10(lp->segtype)) {
			/*
			 * FIXME: When RAID10 is no longer limited to
			 *        2-way mirror, 'lv_mirror_count()'
			 *        must also change for RAID10.
			 */
			log_error("RAID10 currently supports "
				  "only 2-way mirroring (i.e. '-m 1')");
			return 0;
		}

		if (lp->mirrors == 1) {
			if (seg_is_mirrored(lp)) {
				log_error("--mirrors must be at least 1 with segment type %s.", lp->segtype->name);
				return 0;
			}
			log_print_unless_silent("Redundant mirrors argument: default is 0");
		}
	} else
		/* Default to 2 mirrored areas if '--type mirror|raid1|raid10' */
		lp->mirrors = seg_is_mirrored(lp) ? 2 : 1;

	/* FIMXE: raid10 check has to change once we support data copies and odd numbers of stripes */
	if (seg_is_raid10(lp) && lp->mirrors * lp->stripes > max_images) {
		log_error("Only up to %u stripes in %s supported currently.",
			  max_images / lp->mirrors, lp->segtype->name);
		return 0;
	}

	if (seg_is_mirrored(lp)) {
		if (lp->mirrors > max_images) {
			log_error("Only up to %u mirrors in %s supported currently.",
				  max_images, lp->segtype->name);
			return 0;
		}
	} else if (lp->stripes > max_images) {
		log_error("Only up to %u stripes in %s supported currently.",
			  max_images, lp->segtype->name);
		return 0;
	}

	if ((lp->nosync = arg_is_set(cmd, nosync_ARG)) && seg_is_any_raid6(lp)) {
		log_error("nosync option prohibited on RAID6.");
		return 0;
	}

	if (!(lp->region_size = arg_uint_value(cmd, regionsize_ARG, 0)) &&
	    ((lp->region_size = get_default_region_size(cmd)) <= 0)) {
		log_error("regionsize in configuration file is invalid.");
		return 0;
	}

	if (seg_is_mirror(lp) && !_read_mirror_params(cmd, lp))
                return_0;

	if (seg_is_raid(lp) && !_read_raid_params(cmd, lp))
		return_0;

	return 1;
}

static int _read_cache_params(struct cmd_context *cmd,
			      struct lvcreate_params *lp)
{
	if (!seg_is_cache(lp) && !seg_is_cache_pool(lp))
		return 1;

	if (!get_cache_params(cmd,
			      &lp->chunk_size,
			      &lp->cache_metadata_format,
			      &lp->cache_mode,
			      &lp->policy_name,
			      &lp->policy_settings))
		return_0;

	return 1;
}

static int _read_activation_params(struct cmd_context *cmd,
				   struct volume_group *vg,
				   struct lvcreate_params *lp)
{
	unsigned pagesize = lvm_getpagesize() >> SECTOR_SHIFT;

	lp->activate = (activation_change_t)
		arg_uint_value(cmd, activate_ARG, CHANGE_AY);

	/* Error when full */
	if (arg_is_set(cmd, errorwhenfull_ARG)) {
		lp->error_when_full = arg_uint_value(cmd, errorwhenfull_ARG, 0);
	} else
		lp->error_when_full =
			seg_can_error_when_full(lp) &&
			find_config_tree_bool(cmd, activation_error_when_full_CFG, NULL);

	/* Read ahead */
	lp->read_ahead = arg_uint_value(cmd, readahead_ARG,
					cmd->default_settings.read_ahead);
	if (lp->read_ahead != DM_READ_AHEAD_AUTO &&
	    lp->read_ahead != DM_READ_AHEAD_NONE &&
	    lp->read_ahead % pagesize) {
		if (lp->read_ahead < pagesize)
			lp->read_ahead = pagesize;
		else
			lp->read_ahead = (lp->read_ahead / pagesize) * pagesize;
		log_warn("WARNING: Overriding readahead to %u sectors, a multiple "
			 "of %uK page size.", lp->read_ahead, pagesize >> 1);
	}

	/* Persistent minor (and major), default 'n' */
	if (!get_and_validate_major_minor(cmd, vg->fid->fmt, &lp->major, &lp->minor))
		return_0;

	if (arg_is_set(cmd, setactivationskip_ARG)) {
		lp->activation_skip |= ACTIVATION_SKIP_SET;
		if (arg_int_value(cmd, setactivationskip_ARG, 0))
			lp->activation_skip |= ACTIVATION_SKIP_SET_ENABLED;
	}

	if (arg_is_set(cmd, ignoreactivationskip_ARG))
		lp->activation_skip |= ACTIVATION_SKIP_IGNORE;

	return 1;
}

static int _lvcreate_params(struct cmd_context *cmd,
			    int argc, char **argv,
			    struct lvcreate_params *lp,
			    struct lvcreate_cmdline_params *lcp)
{
	int contiguous;
	struct arg_value_group_list *current_group;
	const char *segtype_str;
	const char *tag;
	int only_linear = 0;
	int mirror_default_cfg;

	dm_list_init(&lp->tags);
	lp->target_attr = ~0;
	lp->yes = arg_count(cmd, yes_ARG);
	lp->force = (force_t) arg_count(cmd, force_ARG);
	lp->permission = arg_uint_value(cmd, permission_ARG,
					LVM_READ | LVM_WRITE);

	/*
	 * --type is the top most rule
	 *
	 * Ordering of following type tests is IMPORTANT
	 */
	if ((segtype_str = arg_str_value(cmd, type_ARG, NULL))) {
		lp->type = 1;
		if (!strcmp(segtype_str, "linear")) {
			segtype_str = "striped";
			only_linear = 1; /* User requested linear only target */
		}
	/* More estimations from options after shortcuts */
	} else if (arg_is_set(cmd, snapshot_ARG) &&
		   (arg_is_set(cmd, virtualoriginsize_ARG) ||
		   !arg_is_set(cmd, virtualsize_ARG)))
		/* Snapshot has higher priority then thin */
		segtype_str = SEG_TYPE_NAME_SNAPSHOT; /* --thinpool makes thin volume */
	else if (arg_is_set(cmd, cache_ARG) || arg_is_set(cmd, cachepool_ARG))
		segtype_str = SEG_TYPE_NAME_CACHE;
	else if (arg_is_set(cmd, thin_ARG) || arg_is_set(cmd, thinpool_ARG))
		segtype_str = SEG_TYPE_NAME_THIN;
	else if (arg_is_set(cmd, vdo_ARG))
		segtype_str = SEG_TYPE_NAME_VDO;
	else if (arg_is_set(cmd, virtualsize_ARG)) {
		if (arg_is_set(cmd, virtualoriginsize_ARG))
			segtype_str = SEG_TYPE_NAME_SNAPSHOT; /* --virtualoriginsize incompatible with pools */
		else
			segtype_str = find_config_tree_str(cmd, global_sparse_segtype_default_CFG, NULL);
	} else if (arg_uint_value(cmd, mirrors_ARG, 0)) {
		/* Remember, '-m 0' implies stripe */
		mirror_default_cfg = (arg_uint_value(cmd, stripes_ARG, 1) > 1)
			? global_raid10_segtype_default_CFG : global_mirror_segtype_default_CFG;
		segtype_str = find_config_tree_str(cmd, mirror_default_cfg, NULL);
	} else
		segtype_str = SEG_TYPE_NAME_STRIPED;

	if (!(lp->segtype = get_segtype_from_string(cmd, segtype_str)))
		return_0;

	if (seg_unknown(lp)) {
		log_error("Unable to create LV with unknown segment type %s.", segtype_str);
		return 0;
	}

	/* Starts basic option validation for every segment type */

	/* FIXME Use these ARGS macros also in commands.h? */
	/* ARGS are disjoint! sets of options */
#define LVCREATE_ARGS \
	activate_ARG,\
	addtag_ARG,\
	alloc_ARG,\
	autobackup_ARG,\
	available_ARG,\
	contiguous_ARG,\
	ignoreactivationskip_ARG,\
	ignoremonitoring_ARG,\
	metadataprofile_ARG,\
	monitor_ARG,\
	mirrors_ARG,\
	name_ARG,\
	noudevsync_ARG,\
	permission_ARG,\
	persistent_ARG,\
	readahead_ARG,\
	setactivationskip_ARG,\
	test_ARG,\
	type_ARG

#define CACHE_POOL_ARGS \
	cachemetadataformat_ARG,\
	cachemode_ARG,\
	cachepool_ARG,\
	cachepolicy_ARG,\
	cachesettings_ARG

#define MIRROR_ARGS \
	corelog_ARG,\
	mirrorlog_ARG

#define MIRROR_RAID_ARGS \
	nosync_ARG,\
	regionsize_ARG

#define PERSISTENT_ARGS \
	major_ARG,\
	minor_ARG

#define POOL_ARGS \
	pooldatasize_ARG,\
	poolmetadatasize_ARG,\
	poolmetadataspare_ARG

#define RAID_ARGS \
	maxrecoveryrate_ARG,\
	minrecoveryrate_ARG,\
	raidmaxrecoveryrate_ARG,\
	raidminrecoveryrate_ARG

#define SIZE_ARGS \
	extents_ARG,\
	size_ARG,\
	stripes_ARG,\
	stripesize_ARG

#define THIN_POOL_ARGS \
	discards_ARG,\
	thinpool_ARG

#define VDO_POOL_ARGS \
	compression_ARG,\
	deduplication_ARG

	/* Cache and cache-pool segment type */
	if (seg_is_cache(lp)) {
		/* Only supported with --type cache, -H, --cache */
		if (arg_outside_list_is_set(cmd, "is unsupported with cache",
					    CACHE_POOL_ARGS,
					    LVCREATE_ARGS,
					    PERSISTENT_ARGS,
					    POOL_ARGS,
					    SIZE_ARGS,
					    cache_ARG,
					    chunksize_ARG,
					    wipesignatures_ARG, zero_ARG,
					    -1))
			return_0;
		lp->create_pool = 1; /* Confirmed when opened VG */
	} else if (seg_is_cache_pool(lp)) {
		if (arg_outside_list_is_set(cmd, "is unsupported with cache pools",
					    CACHE_POOL_ARGS,
					    LVCREATE_ARGS,
					    POOL_ARGS,
					    extents_ARG,
					    size_ARG,
					    cache_ARG,
					    chunksize_ARG,
					    -1))
			return_0;
		if (!(lp->permission & LVM_WRITE)) {
			log_error("Cannot create read-only cache pool.");
			return 0;
		}
		lp->create_pool = 1;
	} else if (arg_from_list_is_set(cmd, "is supported only with cache",
					cache_ARG, CACHE_POOL_ARGS,
					-1))
		return_0;

	/* Snapshot segment type */
	if (seg_is_snapshot(lp)) {
		/* Only supported with --type snapshot, -s, --snapshot */
		if (arg_outside_list_is_set(cmd, "is unsupported with snapshots",
					    LVCREATE_ARGS,
					    PERSISTENT_ARGS,
					    SIZE_ARGS,
					    chunksize_ARG,
					    snapshot_ARG,
					    thinpool_ARG,
					    virtualoriginsize_ARG,
					    virtualsize_ARG,
					    -1))
			return_0;

		/* FIXME Resolve this ambiguous case with --pooldatasize  */
		if (arg_is_set(cmd, thinpool_ARG)) {
			if (lp->type) {
				/* Unsupported with --type snapshot */
				log_error("Snapshot segment type is incompatible with thin pools.");
				return 0;
			}

			if (arg_from_list_is_set(cmd, "is unsupported with snapshots and --thinpool",
						 SIZE_ARGS,
						 chunksize_ARG,
						 virtualoriginsize_ARG,
						 virtualsize_ARG,
						 -1))
				return_0;
		}

		/* Snapshot segment type needs size/extents */
		if (lp->type && !arg_is_set(cmd, size_ARG) && !arg_is_set(cmd, extents_ARG)) {
			log_error("Snapshot segment type requires size or extents.");
			return 0;
		}

		lp->snapshot = 1; /* Free arg is snapshot origin */
	} else if (arg_from_list_is_set(cmd, "is supported only with sparse snapshots",
					virtualoriginsize_ARG,
					-1))
		return_0;

	/* Mirror segment type */
	if (seg_is_mirror(lp)) {
		if (arg_outside_list_is_set(cmd, "is unsupported with mirrors",
					    LVCREATE_ARGS,
					    MIRROR_ARGS,
					    MIRROR_RAID_ARGS,
					    PERSISTENT_ARGS,
					    SIZE_ARGS,
					    wipesignatures_ARG, zero_ARG,
					    -1))
			return_0;
	} else if (arg_from_list_is_set(cmd, "is supported only with mirrors",
					MIRROR_ARGS,
					-1))
		return_0;

	/* Raid segment type */
	if (seg_is_raid(lp)) {
		if (arg_outside_list_is_set(cmd, "is unsupported with raids",
					    LVCREATE_ARGS,
					    MIRROR_RAID_ARGS,
					    PERSISTENT_ARGS,
					    RAID_ARGS,
					    SIZE_ARGS,
					    wipesignatures_ARG, zero_ARG,
					    -1))
			return_0;
	} else if (arg_from_list_is_set(cmd, "is supported only with raids",
					RAID_ARGS,
					-1))
		return_0;

	/* Thin and thin-pool segment type */
	if (seg_is_thin_volume(lp)) {
		/* Only supported with --type thin, -T, --thin, -V */
		if (arg_outside_list_is_set(cmd, "is unsupported with thins",
					    LVCREATE_ARGS,
					    PERSISTENT_ARGS,
					    POOL_ARGS,
					    SIZE_ARGS,
					    THIN_POOL_ARGS,
					    chunksize_ARG,
					    errorwhenfull_ARG,
					    snapshot_ARG,
					    thin_ARG,
					    virtualsize_ARG,
					    wipesignatures_ARG, zero_ARG,
					    -1))
			return_0;

		/* If size/extents given with thin, then we are also creating a thin-pool */
		if (arg_is_set(cmd, size_ARG) || arg_is_set(cmd, extents_ARG)) {
			if (arg_is_set(cmd, pooldatasize_ARG)) {
				log_error("Please specify either size or pooldatasize.");
				return 0;
			}
			lp->create_pool = 1;
		} else if (arg_from_list_is_set(cmd, "is supported only with thin pool creation",
						POOL_ARGS,
						SIZE_ARGS,
						chunksize_ARG,
						discards_ARG,
						errorwhenfull_ARG,
						zero_ARG,
						-1))
			return_0;

		if (!arg_is_set(cmd, virtualsize_ARG)) {
			/* Without virtual size could be creation of thin-pool or snapshot */
			if (lp->create_pool) {
				if (lp->type) {
					log_error("Thin segment type requires --virtualsize.");
					return 0;
				}

				log_debug_metadata("Switching from thin to thin pool segment type.");
				if (!(lp->segtype = get_segtype_from_string(cmd, SEG_TYPE_NAME_THIN_POOL)))
					return_0;
			} else	/* Parse free arg as snapshot origin */
				lp->snapshot = 1;
		} else if (arg_is_set(cmd, snapshot_ARG))
			lp->snapshot = 1;
	} else if (seg_is_thin_pool(lp)) {
		if (arg_outside_list_is_set(cmd, "is unsupported with thin pools",
					    LVCREATE_ARGS,
					    POOL_ARGS,
					    SIZE_ARGS,
					    THIN_POOL_ARGS,
					    chunksize_ARG,
					    zero_ARG,
					    -1))
			return_0;
		if (!(lp->permission & LVM_WRITE)) {
			log_error("Cannot create read-only thin pool.");
			return 0;
		}
		lp->create_pool = 1;
	} else if (!lp->snapshot &&
		   arg_from_list_is_set(cmd, "is supported only with thins",
					thin_ARG, THIN_POOL_ARGS,
					-1))
		return_0;
	else if (seg_is_vdo(lp)) {
		/* Only supported with --type thin, -T, --thin, -V */
		if (arg_outside_list_is_set(cmd, "is unsupported with VDOs",
					    LVCREATE_ARGS,
					    PERSISTENT_ARGS,
					    SIZE_ARGS,
					    VDO_POOL_ARGS,
					    vdo_ARG,
					    virtualsize_ARG,
					    wipesignatures_ARG, zero_ARG,
					    -1))
			return_0;

		/* If size/extents given with thin, then we are also creating a thin-pool */
		if (arg_is_set(cmd, size_ARG) || arg_is_set(cmd, extents_ARG)) {
			if (arg_is_set(cmd, pooldatasize_ARG)) {
				log_error("Please specify either size or pooldatasize.");
				return 0;
			}
			lp->create_pool = 1;
		} else if (arg_from_list_is_set(cmd, "is supported only with VDO pool creation",
						VDO_POOL_ARGS,
						SIZE_ARGS,
						zero_ARG,
						-1))
			return_0;

		// FIXME: prefiling here - this is wrong place
		// but will work for this moment
		if (!fill_vdo_target_params(cmd, &lp->vdo_params, NULL))
			return_0;

		if (arg_is_set(cmd, compression_ARG))
			lp->vdo_params.use_compression =
				arg_int_value(cmd, compression_ARG, 0);

		if (arg_is_set(cmd, deduplication_ARG))
			lp->vdo_params.use_deduplication =
				arg_int_value(cmd, deduplication_ARG, 0);
	}

	/* Check options shared between more segment types */
	if (!seg_is_mirror(lp) && !seg_is_raid(lp)) {
		if (arg_from_list_is_set(cmd, "is supported only with mirrors or raids",
					 nosync_ARG,
					 regionsize_ARG,
					 -1))
			return_0;
		/* Let -m0 pass */
		if (arg_int_value(cmd, mirrors_ARG, 0)) {
			log_error("--mirrors is supported only with mirrors or raids");
			return 0;
		}
	}

	if (!lp->create_pool && !lp->snapshot &&
	    arg_from_list_is_set(cmd, "is supported only with pools and snapshots",
				 chunksize_ARG,
				 -1))
		return_0;

	if (!lp->snapshot && !seg_is_thin_volume(lp) && !seg_is_vdo(lp) &&
	    arg_from_list_is_set(cmd, "is supported only with vdo,  sparse snapshots and thins",
				 virtualsize_ARG,
				 -1))
		return_0;

	if (!seg_can_error_when_full(lp) && !lp->create_pool &&
	    arg_is_set(cmd, errorwhenfull_ARG)) {
		log_error("Segment type %s does not support --errorwhenfull.", lp->segtype->name);
		return 0;
	}

	/* Basic segment type validation finished here */

	if (activation() && lp->segtype->ops->target_present) {
		if (!lp->segtype->ops->target_present(cmd, NULL, &lp->target_attr)) {
			log_error("%s: Required device-mapper target(s) not detected in your kernel.",
				  lp->segtype->name);
			return 0;
		}

		if (segtype_is_any_raid0(lp->segtype) &&
		    !(lp->target_attr & RAID_FEATURE_RAID0)) {
			log_error("RAID module does not support RAID0.");
			return 0;
		}

		if (segtype_is_raid4(lp->segtype) &&
		    !(lp->target_attr & RAID_FEATURE_RAID4)) {
			log_error("RAID module does not support RAID4.");
			return 0;
		}

		if (segtype_is_raid10(lp->segtype) && !(lp->target_attr & RAID_FEATURE_RAID10)) {
			log_error("RAID module does not support RAID10.");
			return 0;
		}
	}

	/* Should we zero/wipe signatures on the lv, default to 'y' */
	lp->zero = arg_int_value(cmd, zero_ARG, 1);

	if (arg_is_set(cmd, wipesignatures_ARG)) {
		/* If -W/--wipesignatures is given on command line directly, respect it. */
		lp->wipe_signatures = arg_int_value(cmd, wipesignatures_ARG, 1);
	} else {
		/*
		 * If -W/--wipesignatures is not given on command line,
		 * look at the allocation/wipe_signatures_when_zeroing_new_lvs
		 * to decide what should be done exactly.
		 */
		if (find_config_tree_bool(cmd, allocation_wipe_signatures_when_zeroing_new_lvs_CFG, NULL))
			lp->wipe_signatures = lp->zero;
		else
			lp->wipe_signatures = 0;
	}

	if (!_lvcreate_name_params(cmd, &argc, &argv, lp) ||
	    !_read_size_params(cmd, lp, lcp) ||
	    !get_stripe_params(cmd, lp->segtype, &lp->stripes, &lp->stripe_size, &lp->stripes_supplied, &lp->stripe_size_supplied) ||
	    (lp->create_pool &&
	     !get_pool_params(cmd, lp->segtype,
			      &lp->pool_metadata_size, &lp->pool_metadata_spare,
			      &lp->chunk_size, &lp->discards, &lp->zero_new_blocks)) ||
	    !_read_cache_params(cmd, lp) ||
	    !_read_mirror_and_raid_params(cmd, lp))
		return_0;

	if (only_linear && lp->stripes > 1) {
		log_error("Cannot use stripes with linear type.");
		return 0;
	}

	if (lp->snapshot && (lp->extents || lcp->size)) {
		lp->chunk_size = arg_uint_value(cmd, chunksize_ARG, 8);
		if (lp->chunk_size < 8 || lp->chunk_size > 1024 ||
		    !is_power_of_2(lp->chunk_size)) {
			log_error("Chunk size must be a power of 2 in the "
				  "range 4K to 512K.");
			return 0;
		}
		log_verbose("Setting chunksize to %s.", display_size(cmd, lp->chunk_size));
	}

	/* Allocation parameters */
	contiguous = arg_int_value(cmd, contiguous_ARG, 0);
	lp->alloc = contiguous ? ALLOC_CONTIGUOUS : ALLOC_INHERIT;
	lp->alloc = (alloc_policy_t) arg_uint_value(cmd, alloc_ARG, lp->alloc);

	if (contiguous && (lp->alloc != ALLOC_CONTIGUOUS)) {
		log_error("Conflicting contiguous and alloc arguments.");
		return 0;
	}

	dm_list_iterate_items(current_group, &cmd->arg_value_groups) {
		if (!grouped_arg_is_set(current_group->arg_values, addtag_ARG))
			continue;

		if (!(tag = grouped_arg_str_value(current_group->arg_values, addtag_ARG, NULL))) {
			log_error("Failed to get tag.");
			return 0;
		}

		if (!str_list_add(cmd->mem, &lp->tags, tag)) {
			log_error("Unable to allocate memory for tag %s.", tag);
			return 0;
		}
	}

	lcp->pv_count = argc;
	lcp->pvs = argv;

	return 1;
}

/*
 * _determine_cache_argument
 * @vg
 * @lp
 *
 * 'lp->pool_name' is set with an LV that could be either the cache_pool name
 * or the origin name of the cached LV which is being created.
 * This function determines which it is and sets 'lp->origin_name' or
 * 'lp->pool_name' appropriately.
 */
static int _determine_cache_argument(struct volume_group *vg,
				     struct lvcreate_params *lp)
{
	struct cmd_context *cmd = vg->cmd;
	struct logical_volume *lv;

	if (!lp->pool_name) {
		lp->pool_name = lp->lv_name;
	} else if ((lv = find_lv(vg, lp->pool_name)) && lv_is_cache_pool(lv)) {
		if (!validate_lv_cache_create_pool(lv))
			return_0;
		/* Pool exists, create cache volume */
		lp->create_pool = 0;
		lp->origin_name = NULL;
	} else if (lv) {
		if (arg_is_set(cmd, cachepool_ARG)) {
			/* Argument of --cachepool has to be a cache-pool */
			log_error("Logical volume %s is not a cache pool.",
				  display_lvname(lv));
			return 0;
		}

		/* Origin exists, create cache pool volume */
		if (!validate_lv_cache_create_origin(lv))
			return_0;

		if (arg_is_set(cmd, permission_ARG) &&
		    ((lp->permission & LVM_WRITE) != (lv->status & LVM_WRITE))) {
			/* Reverting permissions on all error path is very complicated */
			log_error("Change of volume permission is unsupported with cache conversion, use lvchange.");
			return 0;
		}
		/* FIXME How to handle skip flag? */
		if (arg_from_list_is_set(cmd, "is unsupported with cache conversion",
					 stripes_ARG,
					 stripesize_ARG,
					 setactivationskip_ARG,
					 ignoreactivationskip_ARG,
					 -1))
			return_0; /* FIXME */

		/* Put origin into resulting activation state first */
		lv = (struct logical_volume *)lv_lock_holder(lv);

		if (is_change_activating(lp->activate)) {
			if ((lp->activate == CHANGE_AAY) &&
			    !lv_passes_auto_activation_filter(cmd, lv)) {
				log_verbose("Skipping activation of cache origin %s.",
					    display_lvname(lv));
				return 1;

			} else if (vg_is_shared(vg)) {
				if (!lv_active_change(cmd, lv, CHANGE_AEY)) {
					log_error("Cannot activate cache origin %s.",
						   display_lvname(lv));
					return 0;
				}

			} else if (!activate_lv(cmd, lv)) {
				log_error("Cannot activate cache origin %s.",
					  display_lvname(lv));
				return 0;
			}
		} else if (!deactivate_lv(cmd, lv)) {
			log_error("Cannot deactivate activate cache origin %s.",
				  display_lvname(lv));
			return 0;
		}

		/* lp->origin_name is already equal to lp->pool_name */
		lp->pool_name = lp->lv_name; /* --name is cache pool name */
		/*  No zeroing of an existing origin! */
		lp->zero = lp->wipe_signatures = 0;
	} else {
		/* Cache pool and cache volume needs to be created */
		lp->origin_name = NULL;
		/* --pooldatasize is needed here */
		log_error("Ambiguous syntax, please create --type cache-pool %s separately.",
			  lp->pool_name);
		return 0;
	}

	return 1;
}

/*
 * Normal snapshot or thinly-provisioned snapshot?
 */
static int _determine_snapshot_type(struct volume_group *vg,
				    struct lvcreate_params *lp,
				    struct lvcreate_cmdline_params *lcp)
{
	struct logical_volume *origin_lv, *pool_lv = NULL;

	if (!(origin_lv = find_lv(vg, lp->origin_name))) {
		log_error("Snapshot origin LV %s not found in Volume group %s.",
			  lp->origin_name, vg->name);
		return 0;
	}
	if (lp->extents || lcp->size)
		return 1; /* Size specified */

	/* Check if we could make thin snapshot */
	if (lp->pool_name) {
		if (!(pool_lv = find_lv(vg, lp->pool_name))) {
			log_error("Thin pool volume %s not found in Volume group %s.",
				  lp->pool_name, vg->name);
			return 0;
		}

		if (!lv_is_thin_pool(pool_lv)) {
			log_error("Logical volume %s is not a thin pool volume.",
				  display_lvname(pool_lv));
			return 0;
		}
	} else {
		if (!lv_is_thin_volume(origin_lv)) {
			if (!seg_is_thin(lp))
				log_error("Please specify either size or extents with snapshots.");
			else
				log_error("Logical volume %s is not a thin volume. "
					  "Thin snapshot supports only thin origins.",
					  display_lvname(origin_lv));
			return 0;
		}
		/* Origin thin volume without size makes thin segment */
		lp->pool_name = first_seg(origin_lv)->pool_lv->name;
	}

	log_debug_metadata("Switching from snapshot to thin segment type.");
	if (!(lp->segtype = get_segtype_from_string(vg->cmd, SEG_TYPE_NAME_THIN)))
		return_0;
	lp->snapshot = 0;

	return 1;
}

static int _check_raid_parameters(struct volume_group *vg,
				  struct lvcreate_params *lp,
				  struct lvcreate_cmdline_params *lcp)
{
	unsigned devs = lcp->pv_count ? : dm_list_size(&vg->pvs);
	uint64_t page_sectors = lvm_getpagesize() >> SECTOR_SHIFT;
	struct cmd_context *cmd = vg->cmd;
	int old_stripes = !arg_is_set(cmd, stripes_ARG) &&
			  find_config_tree_bool(cmd, allocation_raid_stripe_all_devices_CFG, NULL);

	if (vg->extent_size < page_sectors) {
		log_error("Unable to create RAID LV: requires minimum VG extent size %s",
			  display_size(vg->cmd, page_sectors));
		return 0;
	}

	/*
	 * If we requested the previous behaviour by setting
	 * "allocation/raid_stripe_all_devices = 1" and the
	 * number of devices was not supplied, we can infer
	 * from the PVs given.
	 */
	if (old_stripes && seg_is_raid(lp) && !seg_is_raid1(lp))
		lp->stripes = devs;

	if (seg_is_raid10(lp)) {
		lp->stripes /= lp->mirrors;

		if (lp->stripes < 2) {
			log_error("Unable to create RAID(1)0 LV: "
				  "insufficient number of devices.");
			return 0;
		}

	} else if (!seg_is_mirrored(lp)) {
		if (old_stripes &&
		    lp->segtype->parity_devs &&
		    devs > 2 * lp->segtype->parity_devs)
			lp->stripes -= lp->segtype->parity_devs;

		if (seg_is_any_raid0(lp)) {
			if (lp->stripes < 2) {
				log_error("Segment type 'raid0' requires 2 or more stripes.");
				return 0;
			}
		} else if (lp->stripes <= lp->segtype->parity_devs) {
			log_error("Number of stripes must be at least %d for %s",
				  lp->segtype->parity_devs + 1,
				  lp->segtype->name);
			return 0;
		}
	}
	/* 'mirrors' defaults to 2 - not the number of PVs supplied */

	return 1;
}

static int _check_thin_parameters(struct volume_group *vg, struct lvcreate_params *lp,
				  struct lvcreate_cmdline_params *lcp)
{
	if (seg_is_thin_volume(lp) && lp->snapshot) {
		log_error("Please either create snapshot or thin volume.");
		return 0;
	}

	if (!seg_is_thin_volume(lp) && !lp->snapshot) {
		if (!lp->create_pool) {
			/* Not even creating thin pool? */
			log_error("Please specify device size(s).");
			return 0;
		}
	} else if (!lp->create_pool) {
		if (arg_from_list_is_set(vg->cmd, "is only available when creating thin pool",
					 alloc_ARG,
					 chunksize_ARG,
					 contiguous_ARG,
					 stripes_ARG,
					 zero_ARG,
					 -1))
			return_0;

		if (lcp->pv_count) {
			log_error("Only specify Physical volumes when allocating thin pool.");
			return 0;
		}
	}

	return 1;
}

static int _check_pool_parameters(struct cmd_context *cmd,
				  struct volume_group *vg,
				  struct lvcreate_params *lp,
				  struct lvcreate_cmdline_params *lcp)
{
	struct logical_volume *pool_lv;

	if (!lp->create_pool &&
	    arg_from_list_is_set(cmd, "is only available with pools",
				 POOL_ARGS,
				 discards_ARG,
				 -1))
		return_0;

	if (!seg_is_cache(lp) &&
	    !seg_is_thin_volume(lp) &&
	    !seg_is_vdo(lp) &&
	    !seg_is_pool(lp)) {
		if (lp->pool_name && !lp->snapshot) {
			log_error("Segment type %s cannot use pool %s.",
				  lp->segtype->name, lp->pool_name);
			return 0;
		}
		return 1; /* Pool unrelated types */
	}

	if (lp->create_pool) {
		/* Given pool name needs to follow restrictions for created LV */
		if (lp->pool_name) {
			if (!seg_is_cache(lp) && !apply_lvname_restrictions(lp->pool_name))
				return_0;
			/* We could check existance only when we have vg */
			if (vg && find_lv(vg, lp->pool_name)) {
				log_error("Logical volume %s already exists in Volume group %s.",
					  lp->pool_name, vg->name);
				return 0;
			}
		}
		if (seg_is_pool(lp) || seg_is_vdo(lp)) {
			if (lp->major != -1 || lp->minor != -1) {
				log_error("Persistent major and minor numbers are unsupported with pools.");
				return 0;
			}
			/* When creating just pool the pool_name needs to be in lv_name */
			if (seg_is_pool(lp))
				lp->lv_name = lp->pool_name;
		} else if (vg) {
			/* FIXME: what better to do with --readahead and pools? */
			if (arg_is_set(cmd, readahead_ARG)) {
				log_error("Ambigous --readahead parameter specified. Please use either with pool or volume.");
				return 0;
			}
		}

		return 1;
	}
	/* Not creating new pool, but existing pool is needed */
	if (!lp->pool_name) {
		if (lp->snapshot)
			/* Taking snapshot via 'lvcreate -T vg/origin' */
			return 1;
		log_error("Please specify name of existing pool.");
		return 0;
	}

	if (vg) {
		/* Validate pool has matching type */
		if (!(pool_lv = find_lv(vg, lp->pool_name))) {
			log_error("Pool %s not found in Volume group %s.",
				  lp->pool_name, vg->name);
			return 0;
		}
		if (seg_is_cache(lp) && !lv_is_cache_pool(pool_lv)) {
			log_error("Logical volume %s is not a cache pool.",
				  display_lvname(pool_lv));
			return 0;
		}
		if (seg_is_thin_volume(lp) && !lv_is_thin_pool(pool_lv)) {
			log_error("Logical volume %s is not a thin pool.",
				  display_lvname(pool_lv));
			return 0;
		}
	}

	return 1;
}

static int _check_vdo_parameters(struct volume_group *vg, struct lvcreate_params *lp,
				  struct lvcreate_cmdline_params *lcp)
{
	if (seg_is_vdo(lp) && lp->snapshot) {
		log_error("Please either create VDO or snapshot.");
		return 0;
	}

	return 1;
}

/*
 * Check zero_ARG with default value set to value of wipesignatures_ARG
 * with its default set to 'n'. So if user specifies on command line either
 * -Zy or -Wy it will check for incompatible options will report error then.
 *
 * Catching cases like we cannot fulfill:
 *   lvcreate [-an][-pr][-aay][-ky]  [-Zy][-Wy]
 */
static int _check_zero_parameters(struct cmd_context *cmd, struct lvcreate_params *lp)
{
	char buf[NAME_LEN + 128];

	/* -Z has different meaning for thins */
	if (seg_is_thin(lp))
		return 1;

	/* If there is some problem, buffer will not be empty */
	if (dm_snprintf(buf, sizeof(buf), "%s%s%s%s%s%s%s",
			lp->origin_name ? "origin " : "",
			lp->origin_name ? : "",
			lp->origin_name ? " " : "",
			!(lp->permission & LVM_WRITE) ? "read-only " : "",
			!is_change_activating(lp->activate) ? "inactive " : "",
			(lp->activate == CHANGE_AAY) ? "auto activated " : "",
			((lp->activation_skip & ACTIVATION_SKIP_SET_ENABLED) &&
			 !(lp->activation_skip & ACTIVATION_SKIP_IGNORE))
			? "skipped from activation " : "") < 0) {
		log_error(INTERNAL_ERROR "Buffer is too small for dm_snprintf().");
		return 0;
	}

	if (buf[0] || (lp->segtype->flags & SEG_CANNOT_BE_ZEROED)) {
		/* Found condition that prevents zeroing */
		if (arg_int_value(cmd, zero_ARG, arg_int_value(cmd, wipesignatures_ARG, 0))) {
			if (!(lp->segtype->flags & SEG_CANNOT_BE_ZEROED)) {
				log_error("Cannot zero %slogical volume with option -Zy or -Wy.", buf);
				return 0;
			}
			log_print_unless_silent("Ignoring option -Zy or -Wy for unzeroable %s volume.",
						lp->segtype->name);
		}
		lp->zero = lp->wipe_signatures = 0;
	}

	return 1;
}


/*
 * Ensure the set of thin parameters extracted from the command line is consistent.
 */
static int _validate_internal_thin_processing(const struct lvcreate_params *lp)
{
	int r = 1;

	/*
	   The final state should be one of:
	   thin  create_pool  snapshot   origin   pool
	     1        1           0         0      y/n    - create new pool and a thin LV in it
	     1        0           0         0      y      - create new thin LV in existing pool
	     0        1           0         0      y/n    - create new pool only
	     1        0           1         1      y      - create thin snapshot of existing thin LV
	*/

	if (!lp->create_pool && !lp->pool_name) {
		log_error(INTERNAL_ERROR "--thinpool not identified.");
		r = 0;
	}

	if ((!lp->origin_name && lp->snapshot) ||
	    (lp->origin_name && !lp->snapshot && !seg_is_thin_volume(lp))) {
		log_error(INTERNAL_ERROR "Inconsistent snapshot and origin parameters identified.");
		r = 0;
	}

	if (!lp->create_pool && !lp->snapshot && !seg_is_thin_volume(lp)) {
		log_error(INTERNAL_ERROR "Failed to identify what type of thin target to use.");
		r = 0;
	}

	return r;
}

static void _destroy_lvcreate_params(struct lvcreate_params *lp)
{
	if (lp->policy_settings) {
		dm_config_destroy(lp->policy_settings);
		lp->policy_settings = NULL;
	}
}

static int _lvcreate_single(struct cmd_context *cmd, const char *vg_name,
			    struct volume_group *vg, struct processing_handle *handle)
{
	struct processing_params *pp = (struct processing_params *) handle->custom_handle;
	struct lvcreate_params *lp = pp->lp;
	struct lvcreate_cmdline_params *lcp = pp->lcp;
	struct logical_volume *spare = vg->pool_metadata_spare_lv;
	int ret = ECMD_FAILED;

	if (!_read_activation_params(cmd, vg, lp))
		goto_out;

	/* Resolve segment types with opened VG */
	if (lp->snapshot && lp->origin_name && !_determine_snapshot_type(vg, lp, lcp))
		goto_out;

	if (seg_is_cache(lp) && !_determine_cache_argument(vg, lp))
		goto_out;

	/* All types resolved at this point, now only validation steps */
	if (seg_is_raid(lp) && !_check_raid_parameters(vg, lp, lcp))
		goto_out;

	if (seg_is_thin(lp) && !_check_thin_parameters(vg, lp, lcp))
		goto_out;

	if (!_check_pool_parameters(cmd, vg, lp, lcp))
		goto_out;

	if (seg_is_vdo(lp) && !_check_vdo_parameters(vg, lp, lcp))
		return_0;

	/* All types are checked */
	if (!_check_zero_parameters(cmd, lp))
		return_0;

	if (!_update_extents_params(vg, lp, lcp))
		goto_out;

	if (seg_is_thin(lp) && !_validate_internal_thin_processing(lp))
		goto_out;

	if (lp->create_pool && !seg_is_vdo(lp)) {
		/* TODO: VDO does not use spare LV ATM, maybe later for rescue resize ? */
		if (!handle_pool_metadata_spare(vg, lp->pool_metadata_extents,
						lp->pvh, lp->pool_metadata_spare))
			goto_out;

		log_verbose("Making pool %s in VG %s using segtype %s",
			    lp->pool_name ? : "with generated name", lp->vg_name, lp->segtype->name);
	}

	if (vg->lock_type && !strcmp(vg->lock_type, "sanlock")) {
		if (!handle_sanlock_lv(cmd, vg)) {
			log_error("No space for sanlock lock, extend the internal lvmlock LV.");
			goto out;
		}
	}

	if (seg_is_thin_volume(lp))
		log_verbose("Making thin LV %s in pool %s in VG %s%s%s using segtype %s.",
			    lp->lv_name ? : "with generated name",
			    lp->pool_name ? : "with generated name", lp->vg_name,
			    lp->snapshot ? " as snapshot of " : "",
			    lp->snapshot ? lp->origin_name : "", lp->segtype->name);

	if (vg_is_shared(vg)) {
		if (cmd->command->command_enum == lvcreate_thin_vol_with_thinpool_or_sparse_snapshot_CMD) {
			log_error("Use lvconvert to create thin pools and cache pools in a shared VG.");
			goto out;
		}

		lp->needs_lockd_init = 1;
	}

	if (!lv_create_single(vg, lp))
		goto_out;

	ret = ECMD_PROCESSED;
out:
	if (ret != ECMD_PROCESSED && !spare && vg->pool_metadata_spare_lv)
		/* Remove created spare volume for failed pool creation */
		if (!lvremove_single(cmd, vg->pool_metadata_spare_lv, NULL))
			log_error("Removal of created spare volume failed. "
				  "Manual intervention required.");

	return ret;
}

int lvcreate(struct cmd_context *cmd, int argc, char **argv)
{
	struct processing_handle *handle = NULL;
	struct processing_params pp;
	struct lvcreate_params lp = {
		.major = -1,
		.minor = -1,
	};
	struct lvcreate_cmdline_params lcp = { 0 };
	int ret;

	if (!_lvcreate_params(cmd, argc, argv, &lp, &lcp)) {
		stack;
		return EINVALID_CMD_LINE;
	}

	if (!_check_pool_parameters(cmd, NULL, &lp, &lcp)) {
		stack;
		return EINVALID_CMD_LINE;
	}

	pp.lp = &lp;
	pp.lcp = &lcp;

        if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &pp;

	ret = process_each_vg(cmd, 0, NULL, lp.vg_name, NULL, READ_FOR_UPDATE, 0, handle,
			      &_lvcreate_single);

	_destroy_lvcreate_params(&lp);
	destroy_processing_handle(cmd, handle);
	return ret;
}
