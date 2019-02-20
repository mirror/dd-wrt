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
#include "lib/display/display.h"
#include "lib/config/config.h"
#include "lib/datastruct/str_list.h"
#include "lib/activate/activate.h"
#include "lib/datastruct/str_list.h"

static int _errseg_merge_segments(struct lv_segment *seg1, struct lv_segment *seg2)
{
	seg1->len += seg2->len;
	seg1->area_len += seg2->area_len;

	return 1;
}

#ifdef DEVMAPPER_SUPPORT
static int _errseg_add_target_line(struct dev_manager *dm __attribute__((unused)),
				struct dm_pool *mem __attribute__((unused)),
				struct cmd_context *cmd __attribute__((unused)),
				void **target_state __attribute__((unused)),
				struct lv_segment *seg __attribute__((unused)),
				const struct lv_activate_opts *laopts __attribute__((unused)),
				struct dm_tree_node *node, uint64_t len,
				uint32_t *pvmove_mirror_count __attribute__((unused)))
{
	return dm_tree_node_add_error_target(node, len);
}

static int _errseg_target_present(struct cmd_context *cmd,
				  const struct lv_segment *seg __attribute__((unused)),
				  unsigned *attributes __attribute__((unused)))
{
	static int _errseg_checked = 0;
	static int _errseg_present = 0;

	if (!activation())
		return 0;

	/* Reported truncated in older kernels */
	if (!_errseg_checked) {
		_errseg_checked = 1;
		_errseg_present = target_present(cmd, TARGET_NAME_ERROR, 0) ||
			target_present(cmd, TARGET_NAME_ERROR_OLD, 0);
	}

	return _errseg_present;
}

static int _errseg_modules_needed(struct dm_pool *mem,
				  const struct lv_segment *seg __attribute__((unused)),
				  struct dm_list *modules)
{
	if (!str_list_add(mem, modules, MODULE_NAME_ERROR)) {
		log_error("error module string list allocation failed");
		return 0;
	}

	return 1;
}
#endif

static void _errseg_destroy(struct segment_type *segtype)
{
	free(segtype);
}

static struct segtype_handler _error_ops = {
	.merge_segments = _errseg_merge_segments,
#ifdef DEVMAPPER_SUPPORT
	.add_target_line = _errseg_add_target_line,
	.target_present = _errseg_target_present,
	.modules_needed = _errseg_modules_needed,
#endif
	.destroy = _errseg_destroy,
};

struct segment_type *init_error_segtype(struct cmd_context *cmd)
{
	struct segment_type *segtype = zalloc(sizeof(*segtype));

	if (!segtype)
		return_NULL;

	segtype->ops = &_error_ops;
	segtype->name = SEG_TYPE_NAME_ERROR;
	segtype->flags = SEG_CAN_SPLIT | SEG_VIRTUAL | SEG_CANNOT_BE_ZEROED;

	log_very_verbose("Initialised segtype: %s", segtype->name);

	return segtype;
}
