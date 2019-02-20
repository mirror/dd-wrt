/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2017 Red Hat, Inc. All rights reserved.
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

#include "lib/mm/memlock.h"

/*
 * Passed back from callee to request caller to
 * commit and optionally reload metadata.
 *
 * This allows for one metadata update per command run
 * (unless mandatory interim ones in callee).
 */
#define	MR_COMMIT	0x1 /* Commit metadata, don't reload table(s) */
#define	MR_RELOAD	0x2 /* Commit metadata _and_  reload table(s) */

static int _vg_write_commit(const struct logical_volume *lv, const char *what)
{
	log_very_verbose("Updating %s%slogical volume %s on disk(s).",
			 what ? : "", what ? " " : "", display_lvname(lv));
	if (!vg_write(lv->vg) || !vg_commit(lv->vg)) {
		log_error("Failed to update %smetadata of %s on disk.",
			  what ? : "", display_lvname(lv));
		return 0;
	}

	return 1;
}

static int _lvchange_permission(struct cmd_context *cmd,
				struct logical_volume *lv,
				uint32_t *mr)
{
	uint32_t lv_access;
	struct lvinfo info;

	lv_access = arg_uint_value(cmd, permission_ARG, 0);

	if (!(lv_access & LVM_WRITE) && !(lv->status & LVM_WRITE)) {
		/* Refresh if it's read-only in metadata but read-write in kernel */
		if (lv_info(cmd, lv, 0, &info, 0, 0) && info.exists && !info.read_only) {
			log_print_unless_silent("Logical volume %s is already read-only.  Refreshing kernel state.",
						display_lvname(lv));
			return lv_refresh(cmd, lv);
		}
		log_error("Logical volume \"%s\" is already read only.",
			  display_lvname(lv));
		return 0;
	}

	if ((lv_access & LVM_WRITE) && (lv->status & LVM_WRITE)) {
		/* Refresh if it's read-write in metadata but read-only in kernel */
		if (lv_info(cmd, lv, 0, &info, 0, 0) && info.exists && info.read_only) {
			log_print_unless_silent("Logical volume %s is already writable.  Refreshing kernel state.",
						display_lvname(lv));
			return lv_refresh(cmd, lv);
		}

		log_error("Logical volume %s is already writable.",
			  display_lvname(lv));
		return 0;
	}

	if (lv_access & LVM_WRITE) {
		lv->status |= LVM_WRITE;
		log_verbose("Setting logical volume %s read/write.",
			    display_lvname(lv));
	} else {
		lv->status &= ~LVM_WRITE;
		log_verbose("Setting logical volume %s read-only.",
			    display_lvname(lv));
	}

	/* Request caller to commit and reload metadata */
	*mr |= MR_RELOAD;

	return 1;
}

static int _lvchange_pool_update(struct cmd_context *cmd,
				 struct logical_volume *lv,
				 uint32_t *mr)
{
	int update = 0;
	unsigned val;
	thin_discards_t discards;

	if (arg_is_set(cmd, discards_ARG)) {
		discards = (thin_discards_t) arg_uint_value(cmd, discards_ARG, THIN_DISCARDS_IGNORE);
		if (discards != first_seg(lv)->discards) {
			if (((discards == THIN_DISCARDS_IGNORE) ||
			     (first_seg(lv)->discards == THIN_DISCARDS_IGNORE)) &&
			    pool_is_active(lv))
				log_error("Cannot change support for discards while pool volume %s is active.",
					  display_lvname(lv));
			else {
				first_seg(lv)->discards = discards;
				update++;
			}
		} else
			log_error("Logical volume %s already uses --discards %s.",
				  display_lvname(lv), get_pool_discards_name(discards));
	}

	if (arg_is_set(cmd, zero_ARG)) {
		val = arg_uint_value(cmd, zero_ARG, 0) ? THIN_ZERO_YES : THIN_ZERO_NO;
		if (val != first_seg(lv)->zero_new_blocks) {
			first_seg(lv)->zero_new_blocks = val;
			update++;
		} else
			log_error("Logical volume %s already %szero new blocks.",
				  display_lvname(lv), val ? "" : "does not ");
	}

	if (!update)
		return 0;

	/* Request caller to commit and reload metadata */
	*mr |= MR_RELOAD;

	return 1;
}

/*
 * The --monitor y|n value is read from dmeventd_monitor_mode(),
 * which was set by the init_dmeventd_monitor() /
 * get_activation_monitoring_mode() / arg_int_value(monitor_ARG).
 */

static int _lvchange_monitoring(struct cmd_context *cmd,
				struct logical_volume *lv)
{
	struct lvinfo info;

	if (!lv_info(cmd, lv, lv_is_thin_pool(lv) ? 1 : 0,
		     &info, 0, 0) || !info.exists) {
		log_error("Logical volume %s is not active.", display_lvname(lv));
		return 0;
	}

	if (dmeventd_monitor_mode() != DMEVENTD_MONITOR_IGNORE) {
		if (dmeventd_monitor_mode())
			log_verbose("Monitoring LV %s", display_lvname(lv));
		else
			log_verbose("Unmonitoring LV %s", display_lvname(lv));
		if (!monitor_dev_for_events(cmd, lv, 0, dmeventd_monitor_mode()))
			return_0;
	}

	return 1;
}

/*
 * The --poll y|n value is read from background_polling(),
 * which was set by init_background_polling(arg_int_value(poll_ARG)).
 */

static int _lvchange_background_polling(struct cmd_context *cmd,
					struct logical_volume *lv)
{
	struct lvinfo info;

	if (!lv_info(cmd, lv, 0, &info, 0, 0) || !info.exists) {
		log_error("Logical volume %s is not active.", display_lvname(lv));
		return 0;
	}

	if (background_polling()) {
		log_verbose("Polling LV %s", display_lvname(lv));
		lv_spawn_background_polling(cmd, lv);
	}

	return 1;
}

static int _lvchange_activate(struct cmd_context *cmd, struct logical_volume *lv)
{
	activation_change_t activate;

	activate = (activation_change_t) arg_uint_value(cmd, activate_ARG, CHANGE_AY);

	/*
	 * We can get here in the odd case where an LV is already active in
	 * a foreign VG, which allows the VG to be accessed by lvchange -a
	 * so the LV can be deactivated.
	 */
	if (lv->vg->system_id && lv->vg->system_id[0] &&
	    cmd->system_id && cmd->system_id[0] &&
	    strcmp(lv->vg->system_id, cmd->system_id) &&
	    is_change_activating(activate)) {
		log_error("Cannot activate LVs in a foreign VG.");
		return ECMD_FAILED;
	}

	if (lv_activation_skip(lv, activate, arg_is_set(cmd, ignoreactivationskip_ARG)))
		return 1;

	if (lv_is_cow(lv) && !lv_is_virtual_origin(origin_from_cow(lv)))
		lv = origin_from_cow(lv);

	if ((activate == CHANGE_AAY) &&
	    !lv_passes_auto_activation_filter(cmd, lv))
		return 1;

	if (!lv_change_activate(cmd, lv, activate))
		return_0;

	/*
	 * FIXME: lvchange should defer background polling in a similar
	 * 	  way as vgchange does. First activate all relevant LVs
	 * 	  initate background polling later (for all actually
	 * 	  activated LVs). So we can avoid duplicate background
	 * 	  polling for pvmove (2 or more locked LVs on single pvmove
	 * 	  LV)
	 */
	if (background_polling() && is_change_activating(activate) &&
	    (lv_is_pvmove(lv) || lv_is_locked(lv) || lv_is_converting(lv) ||
	     lv_is_merging(lv)))
		lv_spawn_background_polling(cmd, lv);

	return 1;
}

static int _detach_metadata_devices(struct lv_segment *seg, struct dm_list *list)
{
	uint32_t s;
	uint32_t num_meta_lvs;
	struct lv_list *lvl;

	num_meta_lvs = seg_is_raid(seg) ? seg->area_count : !!seg->log_lv;

	if (!num_meta_lvs)
		return_0;

	if (!(lvl = dm_pool_alloc(seg->lv->vg->vgmem, sizeof(*lvl) * num_meta_lvs)))
		return_0;

	if (seg_is_raid_with_meta(seg)) {
		for (s = 0; s < seg->area_count; s++) {
			if (!seg_metalv(seg, s))
				return_0; /* Trap this future possibility */

			lvl[s].lv = seg_metalv(seg, s);
			lv_set_visible(lvl[s].lv);

			dm_list_add(list, &lvl[s].list);
		}
		return 1;
	}

	lvl[0].lv = detach_mirror_log(seg);
	dm_list_add(list, &lvl[0].list);

	return 1;
}

static int _attach_metadata_devices(struct lv_segment *seg, struct dm_list *list)
{
	struct lv_list *lvl;

	if (seg_is_raid(seg)) {
		dm_list_iterate_items(lvl, list)
			lv_set_hidden(lvl->lv);
		return 1;
	}

	dm_list_iterate_items(lvl, list)
		break;  /* get first item */

	if (!attach_mirror_log(seg, lvl->lv))
		return_0;

	return 1;
}

static int _reactivate_lv(struct logical_volume *lv,
			  int active, int exclusive)
{
	struct cmd_context *cmd = lv->vg->cmd;

	if (!active)
		return 1;

	return activate_lv(cmd, lv);
}

/*
 * lvchange_resync
 * @cmd
 * @lv
 *
 * Force a mirror or RAID array to undergo a complete initializing resync.
 */
static int _lvchange_resync(struct cmd_context *cmd, struct logical_volume *lv)
{
	int active = 0;
	int exclusive = 0;
	int monitored;
	struct lv_segment *seg = first_seg(lv);
	struct dm_list device_list;

	dm_list_init(&device_list);

	if (lv_is_active(lv)) {
		if (!lv_check_not_in_use(lv, 1)) {
			log_error("Can't resync open logical volume %s.",
				  display_lvname(lv));
			return 0;
		}

		if (!arg_is_set(cmd, yes_ARG) &&
		    yes_no_prompt("Do you really want to deactivate "
				  "logical volume %s to resync it? [y/n]: ",
				  display_lvname(lv)) == 'n') {
			log_error("Logical volume %s not resynced.",
				  display_lvname(lv));
			return 0;
		}

		active = 1;
		if (lv_is_active(lv))
			exclusive = 1;
	}

	if (seg_is_raid(seg) && active && !exclusive) {
		log_error("RAID logical volume %s cannot be active remotely.",
			  display_lvname(lv));
		return 0;
	}

	/* Activate exclusively to ensure no nodes still have LV active */
	monitored = dmeventd_monitor_mode();
	if (monitored != DMEVENTD_MONITOR_IGNORE)
		init_dmeventd_monitor(0);

	if (!deactivate_lv(cmd, lv)) {
		log_error("Unable to deactivate %s for resync.", display_lvname(lv));
		return 0;
	}

	if (monitored != DMEVENTD_MONITOR_IGNORE)
		init_dmeventd_monitor(monitored);
	init_mirror_in_sync(0);

	log_very_verbose("Starting resync of %s%s%s%s %s.",
			 (active) ? "active " : "",
			 vg_is_clustered(lv->vg) ? "clustered " : "",
			 (seg->log_lv) ? "disk-logged " :
			 seg_is_raid(seg) ? "" : "core-logged ",
			 lvseg_name(seg), display_lvname(lv));

	/*
	 * If this mirror has a core log (i.e. !seg->log_lv),
	 * then simply deactivating/activating will cause
	 * it to reset the sync status.  We only need to
	 * worry about persistent logs.
	 */
	if (!seg_is_raid(seg) && !seg->log_lv) {
		if (lv_is_not_synced(lv)) {
			lv->status &= ~LV_NOTSYNCED;
			if (!_vg_write_commit(lv, NULL))
				return 0;
		}

		if (!_reactivate_lv(lv, active, exclusive)) {
			log_error("Failed to reactivate %s to resynchronize mirror.",
				  display_lvname(lv));
			return 0;
		}

		return 1;
	}

	/*
	 * Now we handle mirrors with log devices
	 */
	lv->status &= ~LV_NOTSYNCED;
	lv->status |= LV_ACTIVATION_SKIP;

	/* Separate mirror log or metadata devices so we can clear them */
	if (!_detach_metadata_devices(seg, &device_list)) {
		log_error("Failed to clear %s %s for %s.",
			  lvseg_name(seg), seg_is_raid(seg) ?
			  "metadata area" : "mirror log", display_lvname(lv));
		return 0;
	}

	if (!_vg_write_commit(lv, "intermediate")) {
		if (!_reactivate_lv(lv, active, exclusive))
			stack;
		return 0;
	}

	/* No backup for intermediate metadata, so just unlock memory */
	memlock_unlock(lv->vg->cmd);

	if (!activate_and_wipe_lvlist(&device_list, 0))
		return 0;

	/* Wait until devices are away */
	if (!sync_local_dev_names(lv->vg->cmd)) {
		log_error("Failed to sync local devices after updating %s.",
			  display_lvname(lv));
		return 0;
	}

	/* Put metadata sub-LVs back in place */
	if (!_attach_metadata_devices(seg, &device_list)) {
		log_error("Failed to reattach %s device after clearing.",
			  (seg_is_raid(seg)) ? "metadata" : "log");
		return 0;
	}

	lv->status &= ~LV_ACTIVATION_SKIP;

	if (!_vg_write_commit(lv, NULL))
		return 0;

	if (!_reactivate_lv(lv, active, exclusive)) {
		backup(lv->vg);
		log_error("Failed to reactivate %s after resync.",
			  display_lvname(lv));
		return 0;
	}

	backup(lv->vg);

	return 1;
}

static int _lvchange_alloc(struct cmd_context *cmd,
			   struct logical_volume *lv,
			   uint32_t *mr)
{
	int want_contiguous = arg_int_value(cmd, contiguous_ARG, 0);
	alloc_policy_t alloc = (alloc_policy_t)
		arg_uint_value(cmd, alloc_ARG, (want_contiguous)
			       ? ALLOC_CONTIGUOUS : ALLOC_INHERIT);

	if (alloc == lv->alloc) {
		log_error("Allocation policy of logical volume %s is already %s.",
			  display_lvname(lv), get_alloc_string(alloc));
		return 0;
	}

	lv->alloc = alloc;

	/* FIXME If contiguous, check existing extents already are */

	log_verbose("Setting contiguous allocation policy for %s to %s.",
		    display_lvname(lv), get_alloc_string(alloc));

	log_very_verbose("Updating logical volume %s on disk(s).", display_lvname(lv));

	/* No need to suspend LV for this change */

	/* Request caller to commit metadata */
	*mr |= MR_COMMIT;

	return 1;
}

static int _lvchange_errorwhenfull(struct cmd_context *cmd,
				   struct logical_volume *lv,
				   uint32_t *mr)
{
	unsigned ewf = arg_int_value(cmd, errorwhenfull_ARG, 0);

	if (ewf == lv_is_error_when_full(lv)) {
		log_error("Error when full is already %sset for %s.",
			  (ewf) ? "" : "un", display_lvname(lv));
		return 0;
	}

	if (ewf)
		lv->status |= LV_ERROR_WHEN_FULL;
	else
		lv->status &= ~LV_ERROR_WHEN_FULL;

	/* Request caller to commit and reload metadata */
	*mr |= MR_RELOAD;

	return 1;
}

static int _lvchange_readahead(struct cmd_context *cmd,
			       struct logical_volume *lv,
			       uint32_t *mr)
{
	unsigned read_ahead = 0;
	unsigned pagesize = (unsigned) lvm_getpagesize() >> SECTOR_SHIFT;

	read_ahead = arg_uint_value(cmd, readahead_ARG, 0);

	if (read_ahead != DM_READ_AHEAD_AUTO &&
	    (lv->vg->fid->fmt->features & FMT_RESTRICTED_READAHEAD) &&
	    (read_ahead < 2 || read_ahead > 120)) {
		log_error("Metadata only supports readahead values between 2 and 120.");
		return 0;
	}

	if (read_ahead != DM_READ_AHEAD_AUTO &&
	    read_ahead != DM_READ_AHEAD_NONE && read_ahead % pagesize) {
		if (read_ahead < pagesize)
			read_ahead = pagesize;
		else
			read_ahead = (read_ahead / pagesize) * pagesize;
		log_warn("WARNING: Overriding readahead to %u sectors, a multiple "
			    "of %uK page size.", read_ahead, pagesize >> 1);
	}

	if (lv->read_ahead == read_ahead) {
		if (read_ahead == DM_READ_AHEAD_AUTO)
			log_error("Read ahead is already auto for %s.",
				  display_lvname(lv));
		else
			log_error("Read ahead is already %u for %s.",
				  read_ahead, display_lvname(lv));
		return 0;
	}

	lv->read_ahead = read_ahead;

	log_verbose("Setting read ahead to %u for %s.",
		    read_ahead, display_lvname(lv));

	/* Request caller to commit and reload metadata */
	*mr |= MR_RELOAD;

	return 1;
}

static int _lvchange_persistent(struct cmd_context *cmd,
				struct logical_volume *lv,
				uint32_t *mr)
{
	enum activation_change activate = CHANGE_AN;

	/* The LV lock in lvmlockd should remain as it is. */
	cmd->lockd_lv_disable = 1;

	if (!get_and_validate_major_minor(cmd, lv->vg->fid->fmt,
					  &lv->major, &lv->minor))
		return_0;

	if (lv->minor == -1) {
		if (!(lv->status & FIXED_MINOR)) {
			log_error("Minor number is already not persistent for %s.",
				  display_lvname(lv));
			return 0;
		}
		lv->status &= ~FIXED_MINOR;
		log_verbose("Disabling persistent device number for %s.",
			    display_lvname(lv));
	} else {
		if (lv_is_active(lv)) {
			if (!arg_is_set(cmd, force_ARG) &&
			    !arg_is_set(cmd, yes_ARG) &&
			    yes_no_prompt("Logical volume %s will be "
					  "deactivated temporarily. "
					  "Continue? [y/n]: ",
					  display_lvname(lv)) == 'n') {
				log_error("%s device number not changed.",
					  display_lvname(lv));
				return 0;
			}

			activate = CHANGE_AEY;
		}

		/* Ensuring LV is not active */
		if (!deactivate_lv(cmd, lv)) {
			log_error("Cannot deactivate %s.", display_lvname(lv));
			return 0;
		}
		lv->status |= FIXED_MINOR;
		log_verbose("Setting persistent device number to (%d, %d) for %s.",
			    lv->major, lv->minor, display_lvname(lv));
	}

	if (!_vg_write_commit(lv, NULL))
		return 0;

	if (activate != CHANGE_AN) {
		log_verbose("Re-activating logical volume %s.", display_lvname(lv));
		if (!lv_active_change(cmd, lv, activate)) {
			log_error("%s: reactivation failed.", display_lvname(lv));
			backup(lv->vg);
			return 0;
		}
	}

	return 1;
}

static int _lvchange_cache(struct cmd_context *cmd,
			   struct logical_volume *lv,
			   uint32_t *mr)
{
	cache_metadata_format_t format;
	cache_mode_t mode;
	const char *name;
	struct dm_config_tree *settings = NULL;
	struct lv_segment *pool_seg = first_seg(lv);
	int r = 0, is_clean;
	uint32_t chunk_size = 0; /* FYI: lvchange does NOT support its change */

	if (lv_is_cache(lv))
		pool_seg = first_seg(pool_seg->pool_lv);

	if (!get_cache_params(cmd, &chunk_size, &format, &mode, &name, &settings))
		goto_out;

	if ((mode != CACHE_MODE_UNSELECTED) &&
	    (mode != pool_seg->cache_mode) &&
	    lv_is_cache(lv)) {
		if (!lv_cache_wait_for_clean(lv, &is_clean))
			return_0;
		if (!is_clean) {
			log_error("Cache %s is not clean, refusing to switch cache mode.",
				  display_lvname(lv));
			return 0;
		}
	}

	if (mode && !cache_set_cache_mode(first_seg(lv), mode))
		goto_out;

	if ((name || settings) &&
	    !cache_set_policy(first_seg(lv), name, settings))
		goto_out;

	/* Request caller to commit and reload metadata */
	*mr |= MR_RELOAD;

	r = 1;
out:
	if (settings)
		dm_config_destroy(settings);

	return r;
}

static int _lvchange_tag(struct cmd_context *cmd, struct logical_volume *lv,
			 int arg, uint32_t *mr)
{
	if (!change_tag(cmd, NULL, lv, NULL, arg))
		return_0;

	log_very_verbose("Updating logical volume %s on disk(s).", display_lvname(lv));

	/* No need to suspend LV for this change */

	/* Request caller to commit and reload metadata */
	*mr |= MR_COMMIT;

	return 1;
}

static int _lvchange_rebuild(struct logical_volume *lv)
{
	int pv_count, i = 0;
	char **rebuild_pvs;
	const char *tmp_str;
	struct dm_list *rebuild_pvh = NULL;
	struct arg_value_group_list *group;
	struct volume_group *vg = lv->vg;
	struct cmd_context *cmd = vg->cmd;

	if (!(pv_count = arg_count(cmd, rebuild_ARG))) {
		log_error("No --rebuild found!");
		return 0;
	}

	if (!arg_is_set(cmd, yes_ARG) &&
	    yes_no_prompt("Do you really want to rebuild %u PVs "
			  "of logical volume %s [y/n]: ",
			  pv_count, display_lvname(lv)) == 'n') {
		log_error("Logical volume %s not rebuild.",
			  display_lvname(lv));
		return 0;
	}

	/* rebuild can be specified more than once */
	if (!(rebuild_pvs = dm_pool_alloc(vg->vgmem, sizeof(char *) * pv_count)))
		return_0;

	dm_list_iterate_items(group, &cmd->arg_value_groups) {
		if (!grouped_arg_is_set(group->arg_values, rebuild_ARG))
			continue;

		if (!(tmp_str = grouped_arg_str_value(group->arg_values,
						      rebuild_ARG, NULL)))
			return_0;

		if (!(rebuild_pvs[i++] = dm_pool_strdup(cmd->mem, tmp_str)))
                               return_0;
	}

	if (!(rebuild_pvh = create_pv_list(cmd->mem, vg,
					   pv_count, rebuild_pvs, 0)))
		return_ECMD_FAILED;

	/* Rebuild PVs listed on @rebuild_pvh */
	return lv_raid_rebuild(lv, rebuild_pvh);
}

static int _lvchange_writemostly(struct logical_volume *lv,
				 uint32_t *mr)
{
	int pv_count, i = 0;
	uint32_t s, writemostly;
	char **pv_names;
	const char *tmp_str;
	size_t tmp_str_len;
	struct pv_list *pvl;
	struct arg_value_group_list *group;
	struct cmd_context *cmd = lv->vg->cmd;
	struct lv_segment *raid_seg = first_seg(lv);

	/*
	 * Prohibit writebehind and writebehind during synchronization.
	 *
	 * FIXME: we can do better once we can distingush between
	 *        an initial sync after a linear -> raid1 upconversion
	 *        and any later additions of legs, requested resyncs
	 *        via lvchange or leg repairs/replacements.
	 */
	if (!lv_raid_in_sync(lv)) {
		log_error("Unable to change write%s on %s while it is not in-sync.",
			  arg_is_set(cmd, writemostly_ARG) ? "mostly" : "behind",
			  display_lvname(lv));
		return 0;
	}

	if (arg_is_set(cmd, writebehind_ARG))
		raid_seg->writebehind = arg_uint_value(cmd, writebehind_ARG, 0);

	if ((pv_count = arg_count(cmd, writemostly_ARG))) {
		/* writemostly can be specified more than once */
		pv_names = dm_pool_alloc(lv->vg->vgmem, sizeof(char *) * pv_count);
		if (!pv_names)
			return_0;

		dm_list_iterate_items(group, &cmd->arg_value_groups) {
			if (!grouped_arg_is_set(group->arg_values,
						writemostly_ARG))
				continue;

			if (!(tmp_str = grouped_arg_str_value(group->arg_values,
							      writemostly_ARG,
							      NULL)))
				return_0;

			/*
			 * Writemostly PV specifications can be:
			 *   <PV>   - Turn on writemostly
			 *   <PV>:t - Toggle writemostly
			 *   <PV>:n - Turn off writemostly
			 *   <PV>:y - Turn on writemostly
			 *
			 * We allocate strlen + 3 to add our own ':{t|n|y}' if
			 * not present plus the trailing '\0'.
			 */
			tmp_str_len = strlen(tmp_str);
			if (!(pv_names[i] = dm_pool_zalloc(lv->vg->vgmem, tmp_str_len + 3)))
				return_0;

			if ((tmp_str_len < 3) ||
			    (tmp_str[tmp_str_len - 2] != ':'))
				/* Default to 'y' if no mode specified */
				sprintf(pv_names[i], "%s:y", tmp_str);
			else
				sprintf(pv_names[i], "%s", tmp_str);
			i++;
		}

		for (i = 0; i < pv_count; i++)
			pv_names[i][strlen(pv_names[i]) - 2] = '\0';

		for (i = 0; i < pv_count; i++) {
			if (!(pvl = find_pv_in_vg(lv->vg, pv_names[i]))) {
				log_error("%s not found in volume group, %s",
					  pv_names[i], lv->vg->name);
				return 0;
			}

			for (s = 0; s < raid_seg->area_count; s++) {
				/*
				 * We don't bother checking the metadata area,
				 * since writemostly only affects the data areas.
				 */
				if (seg_type(raid_seg, s) == AREA_UNASSIGNED)
					continue;

				if (lv_is_on_pv(seg_lv(raid_seg, s), pvl->pv)) {
					if (pv_names[i][strlen(pv_names[i]) + 1] == 'y')
						seg_lv(raid_seg, s)->status |=
							LV_WRITEMOSTLY;
					else if (pv_names[i][strlen(pv_names[i]) + 1] == 'n')
						seg_lv(raid_seg, s)->status &=
							~LV_WRITEMOSTLY;
					else if (pv_names[i][strlen(pv_names[i]) + 1] == 't')
						seg_lv(raid_seg, s)->status ^=
							LV_WRITEMOSTLY;
					else
						return_0;
				}
			}

		}

		/* Only allow a maximum on N-1 images to be set writemostly. */
		writemostly = 0;
		for (s = 0; s < raid_seg->area_count; s++)
			if (seg_lv(raid_seg, s)->status & LV_WRITEMOSTLY)
				writemostly++;

		if (writemostly == raid_seg->area_count) {
			log_error("Can't set all images of %s LV %s to writemostly.",
				  lvseg_name(raid_seg), display_lvname(lv));
			return 0;
		}
	}

	/* Request caller to commit and reload metadata */
	*mr |= MR_RELOAD;

	return 1;
}

static int _lvchange_recovery_rate(struct logical_volume *lv,
				   uint32_t *mr)
{
	struct cmd_context *cmd = lv->vg->cmd;
	struct lv_segment *raid_seg = first_seg(lv);

	if (arg_is_set(cmd, minrecoveryrate_ARG))
		raid_seg->min_recovery_rate =
			arg_uint_value(cmd, minrecoveryrate_ARG, 0) / 2;
	if (arg_is_set(cmd, maxrecoveryrate_ARG))
		raid_seg->max_recovery_rate =
			arg_uint_value(cmd, maxrecoveryrate_ARG, 0) / 2;

	if (raid_seg->max_recovery_rate &&
	    (raid_seg->max_recovery_rate < raid_seg->min_recovery_rate)) {
		log_error("Minimum recovery rate cannot be higher than maximum.");
		return 0;
	}

	/* Request caller to commit and reload metadata */
	*mr |= MR_RELOAD;

	return 1;
}

static int _lvchange_profile(struct logical_volume *lv,
			     uint32_t *mr)
{
	const char *old_profile_name, *new_profile_name;
	struct profile *new_profile;

	old_profile_name = lv->profile ? lv->profile->name : "(inherited)";

	if (arg_is_set(lv->vg->cmd, detachprofile_ARG)) {
		new_profile_name = "(inherited)";
		lv->profile = NULL;
	} else {
		if (arg_is_set(lv->vg->cmd, metadataprofile_ARG))
			new_profile_name = arg_str_value(lv->vg->cmd, metadataprofile_ARG, NULL);
		else
			new_profile_name = arg_str_value(lv->vg->cmd, profile_ARG, NULL);
		if (!(new_profile = add_profile(lv->vg->cmd, new_profile_name, CONFIG_PROFILE_METADATA)))
			return_0;
		lv->profile = new_profile;
	}

	log_verbose("Changing configuration profile for LV %s: %s -> %s.",
		    display_lvname(lv), old_profile_name, new_profile_name);

	/* Request caller to commit metadata */
	*mr |= MR_COMMIT;

	return 1;
}

static int _lvchange_activation_skip(struct logical_volume *lv, uint32_t *mr)
{
	int skip = arg_int_value(lv->vg->cmd, setactivationskip_ARG, 0);

	lv_set_activation_skip(lv, 1, skip);

	log_verbose("Changing activation skip flag to %s for LV %s.",
		    display_lvname(lv), skip ? "enabled" : "disabled");

	/* Request caller to commit+backup metadata */
	*mr |= MR_COMMIT;

	return 1;
}

static int _lvchange_compression(struct logical_volume *lv, uint32_t *mr)
{
	struct cmd_context *cmd = lv->vg->cmd;
	unsigned compression = arg_uint_value(cmd, compression_ARG, 0);
	struct lv_segment *seg = first_seg(lv);

	if (lv_is_vdo(lv))
		seg = first_seg(seg_lv(seg, 0));
	else if (!lv_is_vdo_pool(lv)) {
		log_error("Unable to change compression for non VDO volume %s.",
			  display_lvname(lv));
		return 0;
	}

	if (compression == seg->vdo_params.use_compression) {
		log_error("Logical volume %s already uses --compression %c.",
			  display_lvname(lv), compression ? 'y' : 'n');
		return 0;
	}

	seg->vdo_params.use_compression = compression;

	/* Request caller to commit and reload metadata */
	*mr |= MR_RELOAD;

	return 1;
}

static int _lvchange_deduplication(struct logical_volume *lv, uint32_t *mr)
{
	struct cmd_context *cmd = lv->vg->cmd;
	unsigned deduplication = arg_uint_value(cmd, deduplication_ARG, 0);
	struct lv_segment *seg = first_seg(lv);

	if (lv_is_vdo(lv))
		seg = first_seg(seg_lv(seg, 0));
	else if (!lv_is_vdo_pool(lv)) {
		log_error("Unable to change deduplication for non VDO volume %s.",
			  display_lvname(lv));
		return 0;
	}

	if (deduplication == seg->vdo_params.use_deduplication) {
		log_error("Logical volume %s already uses --deduplication %c.",
			  display_lvname(lv), deduplication ? 'y' : 'n');
		return 0;
	}

	seg->vdo_params.use_deduplication = deduplication;

	/* Request caller to commit and reload metadata */
	*mr |= MR_RELOAD;

	return 1;
}

/* Update and reload or commit and/or backup metadata for @lv as requested by @mr */
static int _commit_reload(struct logical_volume *lv, uint32_t mr)
{
	if (mr & MR_RELOAD) {
		if (!lv_update_and_reload(lv))
			return_0;

	} else if ((mr & MR_COMMIT) &&
		   !_vg_write_commit(lv, NULL))
		return 0;

	return 1;
}

/* Helper: check @opt_num is listed in @opts array */
static int _is_option_listed(int opt_enum, int *options)
{
	int i;

	for (i = 0; options[i] != -1; i++)
		if (opt_enum == options[i])
			return 1;
	return 0;
}

/* Check @opt_enum is an option allowing group commit/reload */
static int _option_allows_group_commit(int opt_enum)
{
	int options[] = {
		permission_ARG,
		alloc_ARG,
		contiguous_ARG,
		compression_ARG,
		deduplication_ARG,
		errorwhenfull_ARG,
		readahead_ARG,
		persistent_ARG,
		addtag_ARG,
		deltag_ARG,
		writemostly_ARG,
		writebehind_ARG,
		minrecoveryrate_ARG,
		maxrecoveryrate_ARG,
		profile_ARG,
		metadataprofile_ARG,
		detachprofile_ARG,
		setactivationskip_ARG,
		-1
	};

	return _is_option_listed(opt_enum, options);
}

/* Check @opt_enum requires direct commit/reload */
static int _option_requires_direct_commit(int opt_enum)
{
	int options[] = {
		discards_ARG,
		zero_ARG,
		cachemode_ARG,
		cachepolicy_ARG,
		cachesettings_ARG,
		-1
	};

	return _is_option_listed(opt_enum, options);
}

/*
 * For each lvchange command definintion:
 *
 * lvchange_foo_cmd(cmd, argc, argv);
 * . set cmd fields that apply to "foo"
 * . set any other things that affect behavior of process_each
 * . process_each_lv(_lvchange_foo_single);
 *
 * _lvchange_foo_single(lv);
 * . _lvchange_foo(lv);
 * . (or all the code could live in the _single fn)
 */

/*
 * Process 2 types of options differently
 * minimizing metadata commits and table reloads:
 *
 * 1. process group of options not requiring metadata commit(, reload)
 *    for each option and commit(, reload) metadata for the whole group
 *
 * 2. process the options requiring metadata commit+reload per option
 */
static int _lvchange_properties_single(struct cmd_context *cmd,
			               struct logical_volume *lv,
			               struct processing_handle *handle)
{
	int docmds = 0, doit = 0, doit_total = 0, change_msg = 1, second_group = 0;
	int i, opt_enum;
	uint32_t mr = 0;

	/*
	 * We do not acquire an lvmlockd lock on the LV here because these are
	 * VG metadata changes that do not conflict with the LV being active on
	 * another host.
	 */

	/* First group of options which allow for one metadata commit/update for the whole group */
	for (i = 0; i < cmd->command->ro_count; i++) {
		opt_enum = cmd->command->required_opt_args[i].opt;

		if (!arg_is_set(cmd, opt_enum))
			continue;

		/*
		 * Skip options requiring direct commit/reload
		 * to process them in the second step.
		 */
		if (_option_requires_direct_commit(opt_enum)) {
			second_group++;
			continue;
		}

		/* Archive will only happen once per run. */
		if (!archive(lv->vg))
			return_ECMD_FAILED;

		/*
		 * Process the following options and to a single
		 * metadata commit/reload for the whole group.
		 */
		switch (opt_enum) {
		case permission_ARG:
			docmds++;
			doit += _lvchange_permission(cmd, lv, &mr);
			break;

		case alloc_ARG:
		case contiguous_ARG:
			docmds++;
			doit += _lvchange_alloc(cmd, lv, &mr);
			break;

		case errorwhenfull_ARG:
			docmds++;
			doit += _lvchange_errorwhenfull(cmd, lv, &mr);
			break;

		case readahead_ARG:
			docmds++;
			doit += _lvchange_readahead(cmd, lv, &mr);
			break;

		case persistent_ARG:
			docmds++;
			doit += _lvchange_persistent(cmd, lv, &mr);
			break;

		case addtag_ARG:
		case deltag_ARG:
			docmds++;
			doit += _lvchange_tag(cmd, lv, opt_enum, &mr);
			break;

		case writemostly_ARG:
		case writebehind_ARG:
			docmds++;
			doit += _lvchange_writemostly(lv, &mr);
			break;

		case minrecoveryrate_ARG:
		case maxrecoveryrate_ARG:
			docmds++;
			doit += _lvchange_recovery_rate(lv, &mr);
			break;

		case profile_ARG:
		case metadataprofile_ARG:
		case detachprofile_ARG:
			docmds++;
			doit += _lvchange_profile(lv, &mr);
			break;

		case setactivationskip_ARG:
			docmds++;
			doit += _lvchange_activation_skip(lv, &mr);
			break;

		case compression_ARG:
			docmds++;
			doit += _lvchange_compression(lv, &mr);
			break;

		case deduplication_ARG:
			docmds++;
			doit += _lvchange_deduplication(lv, &mr);
			break;

		default:
			log_error(INTERNAL_ERROR "Failed to check for option %s",
				  arg_long_option_name(i));
		}
	}

	/* Any options of the first group processed? */
	if (docmds) {
		doit_total = doit;
		doit = 0;

		/* Display any logical volume change */
		if (doit_total) {
			log_print_unless_silent("Logical volume %s changed.", display_lvname(lv));
			change_msg = 0;

			/* Commit(, reload) metadata once for whole processed group of options */
			if (!_commit_reload(lv, mr))
				return_ECMD_FAILED;
		}

		/* Bail out if any processing of an option in the first group failed */
		if (docmds != doit_total)
			return_ECMD_FAILED;

		/* Do backup if processing the first group of options went ok */
		backup(lv->vg);

	} else if (!second_group)
		return_ECMD_FAILED;

	/* Second group of options which need per option metadata commit+reload(s) */
	for (i = 0; i < cmd->command->ro_count; i++) {
		opt_enum = cmd->command->required_opt_args[i].opt;

		if (!arg_is_set(cmd, opt_enum))
			continue;

		/* Skip any of the already processed options which allowed for group commit/reload */
		if (_option_allows_group_commit(opt_enum))
			continue;

		/* Archive will only happen once per run */
		if (!archive(lv->vg))
			return_ECMD_FAILED;

		mr = 0;

		/* Run commit and reload after processing each of the following options */
		switch (opt_enum) {
		case discards_ARG:
		case zero_ARG:
			docmds++;
			doit += _lvchange_pool_update(cmd, lv, &mr);
			break;

		case cachemode_ARG:
		case cachepolicy_ARG:
		case cachesettings_ARG:
			docmds++;
			doit += _lvchange_cache(cmd, lv, &mr);
			break;

		default:
			log_error(INTERNAL_ERROR "Failed to check for option %s",
				  arg_long_option_name(i));
		}

		/* Display any logical volume change unless already displayed in step 1. */
		if (doit && change_msg) {
			log_print_unless_silent("Logical volume %s changed.", display_lvname(lv));
			change_msg = 0;
		}

		/* Commit(,reload) metadata per processed option */
		if (!_commit_reload(lv, mr))
			return_ECMD_FAILED;
	}

	doit_total += doit;

	/* Bail out if no options wwre found or any processing of an option in the second group failed */
	if (!docmds || docmds != doit_total)
		return_ECMD_FAILED;

	/* Do backup if processing the second group of options went ok */
	backup(lv->vg);

	return ECMD_PROCESSED;
}

static int _lvchange_properties_check(struct cmd_context *cmd,
				     struct logical_volume *lv,
				     struct processing_handle *handle,
				     int lv_is_named_arg)
{
	if (!lv_is_visible(lv)) {
		/*
		 * Exceptions where we allow lvchange properties on
		 * a hidden sub lv.
		 *
		 * lv_is_thin_pool_data: e.g. needed when the data sublv
		 * is a cache lv and we need to change cache properties.
		 */
		if (lv_is_thin_pool_data(lv))
			return 1;

		if (lv_is_named_arg)
			log_error("Operation not permitted on hidden LV %s.", display_lvname(lv));
		return 0;
	}

	return 1;
}

int lvchange_properties_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	int ret;

	if (cmd->activate_component) {
		log_error("Cannot change LV properties when activating component LVs.");
		return 0;
	}

	/*
	 * A command def rule allows only some options when LV is partial,
	 * so handles_missing_pvs will only affect those.
	 */
	init_background_polling(arg_is_set(cmd, sysinit_ARG) ? 0 : arg_int_value(cmd, poll_ARG, DEFAULT_BACKGROUND_POLLING));
	cmd->handles_missing_pvs = 1;
	ret = process_each_lv(cmd, argc, argv, NULL, NULL, READ_FOR_UPDATE,
			      NULL, &_lvchange_properties_check, &_lvchange_properties_single);

	if (ret != ECMD_PROCESSED)
		return ret;

	/*
	 * Unfortunately, lvchange has previously allowed changing an LV
	 * property and changing LV activation in a single command.  This was
	 * not a good idea because the behavior/results are hard to predict and
	 * not possible to sensibly describe.  It's also unnecessary.  So, this
	 * is here for the sake of compatibility.
	 *
	 * This is extremely ugly; activation should always be done separately.
	 * This is not the full-featured lvchange capability, just the basic
	 * (the advanced activate options are not provided.)
	 *
	 * FIXME: wrap this in a config setting that we can disable by default
	 * to phase this out?
	 */
	if (arg_is_set(cmd, activate_ARG)) {
		log_warn("WARNING: Combining activation change with other commands is not advised.");
		ret = lvchange_activate_cmd(cmd, argc, argv);

	} else if (arg_is_set(cmd, monitor_ARG) || arg_is_set(cmd, poll_ARG)) {
		ret = lvchange_monitor_poll_cmd(cmd, argc, argv);
	}

	return ret;
}

static int _lvchange_activate_single(struct cmd_context *cmd,
				     struct logical_volume *lv,
				     struct processing_handle *handle)
{
	struct logical_volume *origin;
	char snaps_msg[128];

	/* FIXME: untangle the proper logic for cow / sparse / virtual origin */

	/* If LV is sparse, activate origin instead */
	if (lv_is_cow(lv) && lv_is_virtual_origin(origin = origin_from_cow(lv)))
		lv = origin;

	if (lv_is_cow(lv)) {
		origin = origin_from_cow(lv);
		if (origin->origin_count < 2)
			snaps_msg[0] = '\0';
		else if (dm_snprintf(snaps_msg, sizeof(snaps_msg),
				     " and %u other snapshot(s)",
				     origin->origin_count - 1) < 0) {
			log_error("Failed to prepare message.");
			return ECMD_FAILED;
		}

		if (!arg_is_set(cmd, yes_ARG) &&
		    (yes_no_prompt("Change of snapshot %s will also change its "
				   "origin %s%s. Proceed? [y/n]: ",
				   display_lvname(lv), display_lvname(origin),
				   snaps_msg) == 'n')) {
			log_error("Logical volume %s not changed.", display_lvname(lv));
			return ECMD_FAILED;
		}
	}

	if (!_lvchange_activate(cmd, lv))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

static int _lvchange_activate_check(struct cmd_context *cmd,
				     struct logical_volume *lv,
				     struct processing_handle *handle,
				     int lv_is_named_arg)
{
	if (!lv_is_visible(lv) &&
	    !cmd->activate_component && /* activation of named component LV */
	    ((first_seg(lv)->status & MERGING) || /* merging already started */
	     !cmd->process_component_lvs)) { /* deactivation of a component LV */
		if (lv_is_named_arg)
			log_error("Operation not permitted on hidden LV %s.", display_lvname(lv));
		return 0;
	}

	if (lv_is_vdo_pool(lv) && !lv_is_named_arg)
		return 0;	/* Skip VDO pool processing unless explicitely named */

	return 1;
}

int lvchange_activate_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	int ret;
	int do_activate = is_change_activating((activation_change_t)arg_uint_value(cmd, activate_ARG, CHANGE_AY));

	init_background_polling(arg_is_set(cmd, sysinit_ARG) ? 0 : arg_int_value(cmd, poll_ARG, DEFAULT_BACKGROUND_POLLING));
	cmd->handles_missing_pvs = 1;
	cmd->lockd_vg_default_sh = 1;

	/*
	 * Include foreign VGs that contain active LVs.
	 * That shouldn't happen in general, but if it does by some
	 * mistake, then we want to allow those LVs to be deactivated.
	 */
	cmd->include_active_foreign_vgs = 1;

	/* Allow deactivating if locks fail. */
	if (do_activate)
		cmd->lockd_vg_enforce_sh = 1;

	/* When activating, check if given LV is a component LV */
	if (do_activate) {
		if ((argc == 1) && is_component_lvname(argv[0])) {
			/* With single arg with reserved name prompt for component activation */
			if (arg_is_set(cmd, yes_ARG) ||
			    (yes_no_prompt("Do you want to activate component LV "
					   "in read-only mode? [y/n]: ") == 'y')) {
				log_print_unless_silent("Allowing activation of component LV.");
				cmd->activate_component = 1;
			}

			if (sigint_caught())
				return_ECMD_FAILED;
		}
	} else /* Component LVs might be active, support easy deactivation */
		cmd->process_component_lvs = 1;

	ret = process_each_lv(cmd, argc, argv, NULL, NULL, READ_FOR_UPDATE,
			      NULL, &_lvchange_activate_check, &_lvchange_activate_single);

	if (ret != ECMD_PROCESSED)
		return ret;

	if (arg_is_set(cmd, monitor_ARG) || arg_is_set(cmd, poll_ARG))
		ret = lvchange_monitor_poll_cmd(cmd, argc, argv);

	return ret;
}

static int _lvchange_refresh_single(struct cmd_context *cmd,
				    struct logical_volume *lv,
				    struct processing_handle *handle)
{
	log_verbose("Refreshing logical volume %s (if active).", display_lvname(lv));

	if (!lv_refresh(cmd, lv))
		return_ECMD_FAILED;

	/*
	 * FIXME: In some cases, the lv_refresh() starts polling without
	 * checking poll arg.  Pull that out of lv_refresh.
	 */
	if (arg_is_set(cmd, poll_ARG) &&
	    !_lvchange_background_polling(cmd, lv))
		return_ECMD_FAILED;

	if (arg_is_set(cmd, monitor_ARG) &&
	    !_lvchange_monitoring(cmd, lv))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

static int _lvchange_refresh_check(struct cmd_context *cmd,
				       struct logical_volume *lv,
				       struct processing_handle *handle,
				       int lv_is_named_arg)
{
	if (!lv_is_visible(lv)) {
		if (lv_is_named_arg)
			log_error("Operation not permitted on hidden LV %s.", display_lvname(lv));
		return 0;
	}

	return 1;
}

int lvchange_refresh_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	init_background_polling(arg_is_set(cmd, sysinit_ARG) ? 0 : arg_int_value(cmd, poll_ARG, DEFAULT_BACKGROUND_POLLING));
	cmd->handles_missing_pvs = 1;
	cmd->lockd_vg_default_sh = 1;

	return process_each_lv(cmd, argc, argv, NULL, NULL, 0,
			       NULL, &_lvchange_refresh_check, &_lvchange_refresh_single);
}

static int _lvchange_resync_single(struct cmd_context *cmd,
				    struct logical_volume *lv,
				    struct processing_handle *handle)
{
	/* If LV is inactive here, ensure it's not active elsewhere. */
	if (!lockd_lv(cmd, lv, "ex", 0))
		return_ECMD_FAILED;

	if (!_lvchange_resync(cmd, lv))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

static int _lvchange_resync_check(struct cmd_context *cmd,
				       struct logical_volume *lv,
				       struct processing_handle *handle,
				       int lv_is_named_arg)
{
	if (!lv_is_visible(lv)) {
		if (lv_is_named_arg)
			return 1;
		return 0;
	}

	return 1;
}

int lvchange_resync_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	int ret;

	ret = process_each_lv(cmd, argc, argv, NULL, NULL, READ_FOR_UPDATE,
			      NULL, &_lvchange_resync_check, &_lvchange_resync_single);

	if (ret != ECMD_PROCESSED)
		return ret;

	/*
	 * Unfortunately, lvchange has previously allowed resync and changing
	 * activation to be combined in one command.  activate should be
	 * done separately, but this is here to avoid breaking commands that
	 * used this.
	 *
	 * FIXME: wrap this in a config setting that we can disable by default
	 * to phase this out?
	 */
	if (arg_is_set(cmd, activate_ARG)) {
		log_warn("WARNING: Combining activation change with other commands is not advised.");
		ret = lvchange_activate_cmd(cmd, argc, argv);
	}

	return ret;
}

static int _lvchange_syncaction_single(struct cmd_context *cmd,
				       struct logical_volume *lv,
				       struct processing_handle *handle)
{
	/* If LV is inactive here, ensure it's not active elsewhere. */
	if (!lockd_lv(cmd, lv, "ex", 0))
		return_ECMD_FAILED;

	if (!lv_raid_message(lv, arg_str_value(cmd, syncaction_ARG, NULL)))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

static int _lvchange_syncaction_check(struct cmd_context *cmd,
				       struct logical_volume *lv,
				       struct processing_handle *handle,
				       int lv_is_named_arg)
{
	if (!lv_is_visible(lv)) {
		if (lv_is_named_arg)
			return 1;
		return 0;
	}

	return 1;
}

int lvchange_syncaction_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	return process_each_lv(cmd, argc, argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, &_lvchange_syncaction_check, &_lvchange_syncaction_single);
}

static int _lvchange_rebuild_single(struct cmd_context *cmd,
				    struct logical_volume *lv,
				    struct processing_handle *handle)
{
	/* If LV is inactive here, ensure it's not active elsewhere. */
	if (!lockd_lv(cmd, lv, "ex", 0))
		return_ECMD_FAILED;

	if (!_lvchange_rebuild(lv))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

static int _lvchange_rebuild_check(struct cmd_context *cmd,
				     struct logical_volume *lv,
				     struct processing_handle *handle,
				     int lv_is_named_arg)
{
	if (!lv_is_visible(lv)) {
		if (lv_is_named_arg)
			return 1;
		return 0;
	}

	return 1;
}

int lvchange_rebuild_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	return process_each_lv(cmd, argc, argv, NULL, NULL, READ_FOR_UPDATE,
			       NULL, &_lvchange_rebuild_check, &_lvchange_rebuild_single);
}

static int _lvchange_monitor_poll_single(struct cmd_context *cmd,
				         struct logical_volume *lv,
				         struct processing_handle *handle)
{
	if (arg_is_set(cmd, monitor_ARG) &&
	    !_lvchange_monitoring(cmd, lv))
		return_ECMD_FAILED;

	if (arg_is_set(cmd, poll_ARG) &&
	    !_lvchange_background_polling(cmd, lv))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

static int _lvchange_monitor_poll_check(struct cmd_context *cmd,
				     struct logical_volume *lv,
				     struct processing_handle *handle,
				     int lv_is_named_arg)
{
	if (!lv_is_visible(lv)) {
		if (lv_is_named_arg)
			return 1;
		return 0;
	}

	return 1;
}

int lvchange_monitor_poll_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	init_background_polling(arg_is_set(cmd, sysinit_ARG) ? 0 : arg_int_value(cmd, poll_ARG, DEFAULT_BACKGROUND_POLLING));
	cmd->handles_missing_pvs = 1;
	return process_each_lv(cmd, argc, argv, NULL, NULL, 0,
			       NULL, &_lvchange_monitor_poll_check, &_lvchange_monitor_poll_single);
}

static int _lvchange_persistent_single(struct cmd_context *cmd,
				       struct logical_volume *lv,
				       struct processing_handle *handle)
{
	uint32_t mr = 0;

	/* If LV is inactive here, ensure it's not active elsewhere. */
	if (!lockd_lv(cmd, lv, "ex", 0))
		return_ECMD_FAILED;

	if (!_lvchange_persistent(cmd, lv, &mr))
		return_ECMD_FAILED;

	if (!_commit_reload(lv, mr))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

static int _lvchange_persistent_check(struct cmd_context *cmd,
				     struct logical_volume *lv,
				     struct processing_handle *handle,
				     int lv_is_named_arg)
{
	if (!lv_is_visible(lv)) {
		if (lv_is_named_arg)
			log_error("Operation not permitted on hidden LV %s.", display_lvname(lv));
		return 0;
	}

	return 1;
}

int lvchange_persistent_cmd(struct cmd_context *cmd, int argc, char **argv)
{
	int ret;

	init_background_polling(arg_is_set(cmd, sysinit_ARG) ? 0 : arg_int_value(cmd, poll_ARG, DEFAULT_BACKGROUND_POLLING));
	cmd->handles_missing_pvs = 1;

	ret = process_each_lv(cmd, argc, argv, NULL, NULL, READ_FOR_UPDATE,
			      NULL, &_lvchange_persistent_check, &_lvchange_persistent_single);

	if (ret != ECMD_PROCESSED)
		return ret;

	/* See comment in lvchange_properties about needing to allow these. */
	if (arg_is_set(cmd, activate_ARG)) {
		log_warn("WARNING: Combining activation change with other commands is not advised.");
		ret = lvchange_activate_cmd(cmd, argc, argv);

	} else if (arg_is_set(cmd, monitor_ARG) || arg_is_set(cmd, poll_ARG)) {
		ret = lvchange_monitor_poll_cmd(cmd, argc, argv);
	}

	return ret;
}

int lvchange(struct cmd_context *cmd, int argc, char **argv)
{
	log_error(INTERNAL_ERROR "Missing function for command definition %d:%s.",
		  cmd->command->command_index, cmd->command->command_id);
	return ECMD_FAILED;
}
