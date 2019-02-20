/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2006 Red Hat, Inc. All rights reserved.
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

static int _vgmknodes_single(struct cmd_context *cmd, struct logical_volume *lv,
			     struct processing_handle *handle __attribute__((unused)))
{
	if (arg_is_set(cmd, refresh_ARG) && lv_is_visible(lv))
		if (!lv_refresh(cmd, lv))
			return_ECMD_FAILED;

	if (!lv_mknodes(cmd, lv))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

int vgmknodes(struct cmd_context *cmd, int argc, char **argv)
{
	if (!lv_mknodes(cmd, NULL))
		return_ECMD_FAILED;

	return process_each_lv(cmd, argc, argv, NULL, NULL, LCK_VG_READ, NULL, NULL, &_vgmknodes_single);
}
