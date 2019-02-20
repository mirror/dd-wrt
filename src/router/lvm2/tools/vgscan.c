/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2009 Red Hat, Inc. All rights reserved.
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

static int _vgscan_single(struct cmd_context *cmd, const char *vg_name,
			  struct volume_group *vg,
			  struct processing_handle *handle __attribute__((unused)))
{
	log_print_unless_silent("Found %svolume group \"%s\" using metadata type %s",
				vg_is_exported(vg) ? "exported " : "", vg_name,
				vg->fid->fmt->name);

	check_current_backup(vg);

	return ECMD_PROCESSED;
}

int vgscan(struct cmd_context *cmd, int argc, char **argv)
{
	int maxret, ret;

	if (arg_is_set(cmd, notifydbus_ARG)) {
		if (!lvmnotify_is_supported()) {
			log_error("Cannot notify dbus: lvm is not built with dbus support.");
			return ECMD_FAILED;
		}
		if (!find_config_tree_bool(cmd, global_notify_dbus_CFG, NULL)) {
			log_error("Cannot notify dbus: notify_dbus is disabled in lvm config.");
			return ECMD_FAILED;
		}
		set_pv_notify(cmd);
		set_vg_notify(cmd);
		set_lv_notify(cmd);
		return ECMD_PROCESSED;
	}

	if (!lock_vol(cmd, VG_GLOBAL, LCK_VG_WRITE, NULL)) {
		log_error("Unable to obtain global lock.");
		return ECMD_FAILED;
	}

	if (arg_is_set(cmd, cache_long_ARG)) {
		log_warn("Ignoring vgscan --cache command because lvmetad is no longer used.");
		return ECMD_PROCESSED;
	}

	log_print_unless_silent("Reading all physical volumes.  This may take a while...");

	maxret = process_each_vg(cmd, argc, argv, NULL, NULL, 0, 0, NULL, &_vgscan_single);

	if (arg_is_set(cmd, mknodes_ARG)) {
		ret = vgmknodes(cmd, argc, argv);
		if (ret > maxret)
			maxret = ret;
	}

	unlock_vg(cmd, NULL, VG_GLOBAL);
	return maxret;
}
