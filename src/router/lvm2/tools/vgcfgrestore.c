/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2018 Red Hat, Inc. All rights reserved.
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
#include "device_mapper/all.h"
#include "device_mapper/misc/dm-ioctl.h"

/*
 * Check if there are any active volumes from restored vg_name.
 * We can prompt user, as such operation may make some serious
 * troubles later, when user will try to continue such devices.
 */
static int _check_all_dm_devices(const char *vg_name, unsigned *found)
{
	struct dm_task *dmt;
	struct dm_names *names;
	char vgname_buf[DM_NAME_LEN * 2];
	char *vgname, *lvname, *lvlayer;
	unsigned next = 0;
	int r = 1;

	if (!(dmt = dm_task_create(DM_DEVICE_LIST)))
		return_0;

	if (!dm_task_run(dmt)) {
		r = 0;
		goto_out;
	}

	if (!(names = dm_task_get_names(dmt))) {
		r = 0;
		goto_out;
	}

	if (!names->dev) {
		log_verbose("No devices found.");
		goto out;
	}

	do {
		/* TODO: Do we want to validate UUID LVM- prefix as well ? */
		names = (struct dm_names *)((char *) names + next);
		if (!dm_strncpy(vgname_buf, names->name, sizeof(vgname_buf))) {
			r = 0;
			goto_out;
		}
		vgname = vgname_buf;
		if (!dm_split_lvm_name(NULL, NULL, &vgname, &lvname, &lvlayer)) {
			r = 0;
			goto_out;
		}
		if (strcmp(vgname, vg_name) == 0) {
			log_print("Volume group %s has active volume: %s.", vgname, lvname);
			(*found)++;
		}
		next = names->next;
	} while (next);

out:
	dm_task_destroy(dmt);
	return r;
}

int vgcfgrestore(struct cmd_context *cmd, int argc, char **argv)
{
	const char *vg_name = NULL;
	unsigned found = 0;
	int ret;

	if (argc == 1) {
		vg_name = skip_dev_dir(cmd, argv[0], NULL);
		if (!validate_name(vg_name)) {
			log_error("Volume group name \"%s\" is invalid.", vg_name);
			return EINVALID_CMD_LINE;
		}
	} else if (!(arg_is_set(cmd, list_ARG) && arg_is_set(cmd, file_ARG))) {
		log_error("Please specify a *single* volume group to restore.");
		return EINVALID_CMD_LINE;
	}

	/*
	 * FIXME: overloading the -l arg for now to display a
	 * list of archive files for a particular vg
	 */
	if (arg_is_set(cmd, list_ARG)) {
		if (!(arg_is_set(cmd,file_ARG) ?
			    archive_display_file(cmd,
				arg_str_value(cmd, file_ARG, "")) :
			    archive_display(cmd, vg_name)))
			return_ECMD_FAILED;

		return ECMD_PROCESSED;
	}

	if (!_check_all_dm_devices(vg_name, &found)) {
		log_warn("WARNING: Failed to check for active volumes in volume group \"%s\".", vg_name);
	} else if (found) {
		log_warn("WARNING: Found %u active volume(s) in volume group \"%s\".",
			 found, vg_name);
		log_print("Restoring VG with active LVs, may cause mismatch with its metadata.");
		if (!arg_is_set(cmd, yes_ARG) &&
		    yes_no_prompt("Do you really want to proceed with restore of volume group \"%s\", "
				  "while %u volume(s) are active? [y/n]: ",
				  vg_name, found) == 'n') {
			log_error("Restore aborted.");
			return ECMD_FAILED;
		}
	}

	if (!lock_vol(cmd, VG_ORPHANS, LCK_VG_WRITE, NULL)) {
		log_error("Unable to lock orphans.");
		return ECMD_FAILED;
	}

	if (!lock_vol(cmd, vg_name, LCK_VG_WRITE, NULL)) {
		log_error("Unable to lock volume group %s.", vg_name);
		unlock_vg(cmd, NULL, VG_ORPHANS);
		return ECMD_FAILED;
	}

	lvmcache_label_scan(cmd);

	cmd->handles_unknown_segments = 1;

	if (!(arg_is_set(cmd, file_ARG) ?
	      backup_restore_from_file(cmd, vg_name,
				       arg_str_value(cmd, file_ARG, ""),
				       arg_count(cmd, force_long_ARG)) :
	      backup_restore(cmd, vg_name, arg_count(cmd, force_long_ARG)))) {
		unlock_vg(cmd, NULL, vg_name);
		unlock_vg(cmd, NULL, VG_ORPHANS);
		log_error("Restore failed.");
		ret = ECMD_FAILED;
		goto out;
	}

	ret = ECMD_PROCESSED;
	log_print_unless_silent("Restored volume group %s.", vg_name);

	unlock_vg(cmd, NULL, VG_ORPHANS);
	unlock_vg(cmd, NULL, vg_name);
out:
	return ret;
}
