/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004 Red Hat, Inc. All rights reserved.
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
#include "lib/commands/toolcontext.h"
#include "lib/metadata/segtype.h"

struct segment_type *get_segtype_from_string(struct cmd_context *cmd,
					     const char *str)
{
	struct segment_type *segtype;

	dm_list_iterate_items(segtype, &cmd->segtypes)
		if (!strcmp(segtype->name, str))
			return segtype;

	if (!(segtype = init_unknown_segtype(cmd, str)))
		return_NULL;

	dm_list_add(&cmd->segtypes, &segtype->list);
	log_warn("WARNING: Unrecognised segment type %s", str);

	return segtype;
}

struct segment_type *get_segtype_from_flag(struct cmd_context *cmd, uint64_t flag)
{
	struct segment_type *segtype;

	/* Iterate backwards to provide aliases; e.g. raid5 instead of raid5_ls */
	dm_list_iterate_back_items(segtype, &cmd->segtypes)
		if (flag & segtype->flags)
			return segtype;

	log_error(INTERNAL_ERROR "Unrecognised segment type flag 0x%016" PRIx64, flag);

	return NULL;
}
