/*
 * Copyright (C) 2005-2015 Red Hat, Inc. All rights reserved.
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

#include "lvconvert_poll.h"

int lvconvert_mirror_finish(struct cmd_context *cmd,
			    struct volume_group *vg,
			    struct logical_volume *lv,
			    struct dm_list *lvs_changed __attribute__((unused)))
{
	if (!lv_is_converting(lv))
		return 1;

	if (!collapse_mirrored_lv(lv)) {
		log_error("Failed to remove temporary sync layer.");
		return 0;
	}

	lv->status &= ~CONVERTING;

	if (!lv_update_and_reload(lv))
		return_0;

	log_print_unless_silent("Logical volume %s converted.", display_lvname(lv));

	return 1;
}

/* Swap lvid and LV names */
int swap_lv_identifiers(struct cmd_context *cmd,
			struct logical_volume *a, struct logical_volume *b)
{
	union lvid lvid;
	const char *aname = a->name, *bname = b->name;

	lvid = a->lvid;
	a->lvid = b->lvid;
	b->lvid = lvid;

	/* rename temporarily to 'unused' name */
	if (!lv_rename_update(cmd, a, "pmove_tmeta", 0))
		return_0;
	/* name rename 'b' to unused name of 'a' */
	if (!lv_rename_update(cmd, b, aname, 0))
		return_0;
	/* finish name swapping */
	if (!lv_rename_update(cmd, a, bname, 0))
		return_0;

	return 1;
}

static void _move_lv_attributes(struct logical_volume *to, struct logical_volume *from)
{
	/* Maybe move this code into thin_merge_finish() */
	to->status = from->status; // FIXME maybe some masking ?
	to->alloc = from->alloc;
	to->profile = from->profile;
	to->read_ahead = from->read_ahead;
	to->major = from->major;
	to->minor = from->minor;
	to->timestamp = from->timestamp;
	to->hostname = from->hostname;

	/* Move tags */
	dm_list_init(&to->tags);
	dm_list_splice(&to->tags, &from->tags);

	/* Anything else to preserve? */
}

/* Finalise merging of lv into merge_lv */
int thin_merge_finish(struct cmd_context *cmd,
		      struct logical_volume *merge_lv,
		      struct logical_volume *lv)
{
	if (!swap_lv_identifiers(cmd, merge_lv, lv)) {
		log_error("Failed to swap %s with merging %s.",
			  display_lvname(lv), display_lvname(merge_lv));
		return 0;
	}

	/* Preserve origins' attributes */
	_move_lv_attributes(lv, merge_lv);

	/* Removed LV has to be visible */
	if (!lv_remove_single(cmd, merge_lv, DONT_PROMPT, 1))
		return_0;

	return 1;
}

int lvconvert_merge_finish(struct cmd_context *cmd,
			   struct volume_group *vg,
			   struct logical_volume *lv,
			   struct dm_list *lvs_changed __attribute__((unused)))
{
	struct lv_segment *snap_seg = find_snapshot(lv);

	if (!lv_is_merging_origin(lv)) {
		log_error("Logical volume %s has no merging snapshot.",
			  display_lvname(lv));
		return 0;
	}

	log_print_unless_silent("Merge of snapshot into logical volume %s has finished.",
				display_lvname(lv));

	if (seg_is_thin_volume(snap_seg)) {
		clear_snapshot_merge(lv);

		if (!thin_merge_finish(cmd, lv, snap_seg->lv))
			return_0;

	} else if (!lv_remove_single(cmd, snap_seg->cow, DONT_PROMPT, 0)) {
		log_error("Could not remove snapshot %s merged into %s.",
			  display_lvname(snap_seg->cow), display_lvname(lv));
		return 0;
	}

	return 1;
}

progress_t poll_merge_progress(struct cmd_context *cmd,
			       struct logical_volume *lv,
			       const char *name __attribute__((unused)),
			       struct daemon_parms *parms)
{
	dm_percent_t percent = DM_PERCENT_0;

	if (!lv_is_merging_origin(lv) ||
	    !lv_snapshot_percent(lv, &percent)) {
		log_error("%s: Failed query for merging percentage. Aborting merge.",
			  display_lvname(lv));
		return PROGRESS_CHECK_FAILED;
	} else if (percent == DM_PERCENT_INVALID) {
		log_error("%s: Merging snapshot invalidated. Aborting merge.",
			  display_lvname(lv));
		return PROGRESS_CHECK_FAILED;
	} else if (percent == LVM_PERCENT_MERGE_FAILED) {
		log_error("%s: Merge failed. Retry merge or inspect manually.",
			  display_lvname(lv));
		return PROGRESS_CHECK_FAILED;
	}

	if (parms->progress_display)
		log_print_unless_silent("%s: %s: %s%%", display_lvname(lv), parms->progress_title,
					display_percent(cmd, DM_PERCENT_100 - percent));
	else
		log_verbose("%s: %s: %s%%", display_lvname(lv), parms->progress_title,
			    display_percent(cmd, DM_PERCENT_100 - percent));

	if (percent == DM_PERCENT_0)
		return PROGRESS_FINISHED_ALL;

	return PROGRESS_UNFINISHED;
}

progress_t poll_thin_merge_progress(struct cmd_context *cmd,
				    struct logical_volume *lv,
				    const char *name __attribute__((unused)),
				    struct daemon_parms *parms)
{
	uint32_t device_id;

	if (!lv_thin_device_id(lv, &device_id)) {
		stack;
		return PROGRESS_CHECK_FAILED;
	}

	/*
	 * There is no need to poll more than once,
	 * a thin snapshot merge is immediate.
	 */

	if (device_id != find_snapshot(lv)->device_id) {
		log_error("LV %s is not merged.", display_lvname(lv));
		return PROGRESS_CHECK_FAILED;
	}

	return PROGRESS_FINISHED_ALL; /* Merging happend */
}
