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

static int _lvdisplay_single(struct cmd_context *cmd, struct logical_volume *lv,
			     struct processing_handle *handle __attribute__ ((unused)))
{
	if (!arg_is_set(cmd, all_ARG) && !lv_is_visible(lv))
		return ECMD_PROCESSED;

	if (arg_is_set(cmd, colon_ARG))
		lvdisplay_colons(lv);
	else {
		lvdisplay_full(cmd, lv, NULL);
		if (arg_is_set(cmd, maps_ARG))
			lvdisplay_segments(lv);
	}

	return ECMD_PROCESSED;
}

int lvdisplay(struct cmd_context *cmd, int argc, char **argv)
{
	if (arg_is_set(cmd, columns_ARG)) {
		if (arg_is_set(cmd, colon_ARG) || arg_is_set(cmd, maps_ARG)) {
			log_error("Incompatible options selected");
			return EINVALID_CMD_LINE;
		}
		return lvs(cmd, argc, argv);
	}

	if (arg_is_set(cmd, aligned_ARG) ||
	    arg_is_set(cmd, binary_ARG) ||
	    arg_is_set(cmd, noheadings_ARG) ||
	    arg_is_set(cmd, options_ARG) ||
	    arg_is_set(cmd, separator_ARG) ||
	    arg_is_set(cmd, sort_ARG) ||
	    arg_is_set(cmd, unbuffered_ARG)) {
		log_error("Incompatible options selected.");
		return EINVALID_CMD_LINE;
	}

	if (arg_is_set(cmd, colon_ARG) && arg_is_set(cmd, maps_ARG)) {
		log_error("Options -c and -m are incompatible.");
		return EINVALID_CMD_LINE;
	}

	return process_each_lv(cmd, argc, argv, NULL, NULL, 0, NULL, NULL, &_lvdisplay_single);
}
