/*
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
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
#include "lib/commands/toolcontext.h"
#include "lib/metadata/segtype.h"

static void _freeseg_destroy(struct segment_type *segtype)
{
	free(segtype);
}

static struct segtype_handler _freeseg_ops = {
	.destroy = _freeseg_destroy,
};

struct segment_type *init_free_segtype(struct cmd_context *cmd)
{
	struct segment_type *segtype = zalloc(sizeof(*segtype));

	if (!segtype)
		return_NULL;

	segtype->ops = &_freeseg_ops;
	segtype->name = SEG_TYPE_NAME_FREE;
	segtype->flags = SEG_VIRTUAL | SEG_CANNOT_BE_ZEROED;

	log_very_verbose("Initialised segtype: %s", segtype->name);

	return segtype;
}
