/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2016 Red Hat, Inc. All rights reserved.
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

static int _lvresize_params(struct cmd_context *cmd, int argc, char **argv,
			    struct lvresize_params *lp)
{
	const char *cmd_name = command_name(cmd);
	const char *type_str = arg_str_value(cmd, type_ARG, NULL);

	if (type_str && !(lp->segtype = get_segtype_from_string(cmd, type_str)))
		return_0;

	if (!strcmp(cmd_name, "lvreduce"))
		lp->resize = LV_REDUCE;
	else if (!strcmp(cmd_name, "lvextend"))
		lp->resize = LV_EXTEND;
	else
		lp->resize = LV_ANY;

	lp->sign = lp->poolmetadata_sign = SIGN_NONE;

	if ((lp->use_policies = arg_is_set(cmd, usepolicies_ARG))) {
		/* do nothing; lv_resize will handle --use-policies itself */
		if (arg_from_list_is_set(cmd, NULL,
					 chunksize_ARG, extents_ARG,
					 poolmetadatasize_ARG,
					 regionsize_ARG,
					 size_ARG,
					 stripes_ARG, stripesize_ARG,
					 -1))
			log_print_unless_silent("Ignoring size parameters with --use-policies.");
	} else {
		/*
		 * Allow omission of extents and size if the user has given us
		 * one or more PVs.  Most likely, the intent was "resize this
		 * LV the best you can with these PVs"
		 * If only --poolmetadatasize is specified with list of PVs,
		 * then metadata will be extended there.
		 */
		if ((lp->extents = arg_uint_value(cmd, extents_ARG, 0))) {
			lp->sign = arg_sign_value(cmd, extents_ARG, SIGN_NONE);
			lp->percent = arg_percent_value(cmd, extents_ARG, PERCENT_NONE);
		}

		if (arg_from_list_is_zero(cmd, "may not be zero",
					  chunksize_ARG, extents_ARG,
					  poolmetadatasize_ARG,
					  regionsize_ARG,
					  size_ARG,
					  stripes_ARG, stripesize_ARG,
					  virtualsize_ARG,
					  -1))
			return_0;

		if ((lp->poolmetadata_size = arg_uint64_value(cmd, poolmetadatasize_ARG, 0))) {
			lp->poolmetadata_sign = arg_sign_value(cmd, poolmetadatasize_ARG, SIGN_NONE);
			if (lp->poolmetadata_sign == SIGN_MINUS) {
				log_error("Can't reduce pool metadata size.");
				return 0;
			}
		}

		if ((lp->size = arg_uint64_value(cmd, size_ARG, 0))) {
			lp->sign = arg_sign_value(cmd, size_ARG, SIGN_NONE);
			lp->percent = PERCENT_NONE;
		}

		if (lp->size && lp->extents) {
			log_error("Please specify either size or extents but not both.");
			return 0;
		}

		if (!lp->extents &&
		    !lp->size &&
		    !lp->poolmetadata_size &&
		    (argc >= 2)) {
			lp->extents = 100;
			lp->percent = PERCENT_PVS;
			lp->sign = SIGN_PLUS;
		}
	}

	if (lp->resize == LV_EXTEND && lp->sign == SIGN_MINUS) {
		log_error("Negative argument not permitted - use lvreduce.");
		return 0;
	}

	if (lp->resize == LV_REDUCE &&
	    ((lp->sign == SIGN_PLUS) ||
	     (lp->poolmetadata_sign == SIGN_PLUS))) {
		log_error("Positive sign not permitted - use lvextend.");
		return 0;
	}

	if (!argc) {
		log_error("Please provide the logical volume name.");
		return 0;
	}

	lp->lv_name = argv[0];

	if (!validate_lvname_param(cmd, &lp->vg_name, &lp->lv_name))
		return_0;

	/* Check for $LVM_VG_NAME */
	if (!lp->vg_name && !(lp->vg_name = extract_vgname(cmd, NULL))) {
		log_error("Please specify a logical volume path.");
		return 0;
	}

	if (arg_is_set(cmd, mirrors_ARG)) {
		if (arg_sign_value(cmd, mirrors_ARG, SIGN_NONE) != SIGN_NONE) {
			log_error("Mirrors argument may not be signed.");
			return 0;
		}
		if ((lp->mirrors = arg_uint_value(cmd, mirrors_ARG, 0)))
			lp->mirrors++;
	}

	if ((lp->stripes = arg_uint_value(cmd, stripes_ARG, 0)) &&
	    (arg_sign_value(cmd, stripes_ARG, SIGN_NONE) == SIGN_MINUS)) {
		log_error("Stripes argument may not be negative.");
		return 0;
	}

	if ((lp->stripe_size = arg_uint64_value(cmd, stripesize_ARG, 0)) &&
	    (arg_sign_value(cmd, stripesize_ARG, SIGN_NONE) == SIGN_MINUS)) {
		log_error("Stripesize may not be negative.");
		return 0;
	}

	lp->argc = --argc;
	lp->argv = ++argv;

	lp->alloc = (alloc_policy_t) arg_uint_value(cmd, alloc_ARG, 0);
	lp->yes = arg_is_set(cmd, yes_ARG);
	lp->force = arg_is_set(cmd, force_ARG);
	lp->nofsck = arg_is_set(cmd, nofsck_ARG);
	lp->nosync = arg_is_set(cmd, nosync_ARG);
	lp->resizefs = arg_is_set(cmd, resizefs_ARG);

	return 1;
}

static int _lvresize_single(struct cmd_context *cmd, const char *vg_name,
			    struct volume_group *vg, struct processing_handle *handle)
{
	struct lvresize_params *lp = (struct lvresize_params *) handle->custom_handle;
	struct dm_list *pvh;
	struct logical_volume *lv;
	int ret = ECMD_FAILED;

	/* Does LV exist? */
	if (!(lv = find_lv(vg, lp->lv_name))) {
		log_error("Logical volume %s not found in volume group %s.",
			  lp->lv_name, vg->name);
		goto out;
	}

	if (!(pvh = lp->argc ? create_pv_list(cmd->mem, vg, lp->argc, lp->argv, 1) : &vg->pvs))
		goto_out;

	if (!lv_resize(lv, lp, pvh))
		goto_out;

	ret = ECMD_PROCESSED;
out:
	return ret;
}

int lvresize(struct cmd_context *cmd, int argc, char **argv)
{
	struct processing_handle *handle;
	struct lvresize_params lp = { 0 };
	int ret;

	if (!_lvresize_params(cmd, argc, argv, &lp)) {
		stack;
		return EINVALID_CMD_LINE;
	}

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &lp;

	ret = process_each_vg(cmd, 0, NULL, lp.vg_name, NULL, READ_FOR_UPDATE, 0, handle,
			      &_lvresize_single);

	destroy_processing_handle(cmd, handle);

	return ret;
}
