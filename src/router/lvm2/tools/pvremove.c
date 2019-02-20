/*
 * Copyright (C) 2002-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2013 Red Hat, Inc. All rights reserved.
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

int pvremove(struct cmd_context *cmd, int argc, char **argv)
{
	struct processing_handle *handle;
	struct pvcreate_params pp;
	int ret;

	if (!argc) {
		log_error("Please enter a physical volume path");
		return EINVALID_CMD_LINE;
	}

	pvcreate_params_set_defaults(&pp);

	pp.is_remove = 1;
	pp.force = arg_count(cmd, force_ARG);
	pp.yes = arg_count(cmd, yes_ARG);
	pp.pv_count = argc;
	pp.pv_names = argv;

	/*
	 * Needed to change the set of orphan PVs.
	 * (disable afterward to prevent process_each_pv from doing
	 * a shared global lock since it's already acquired it ex.)
	 */
	if (!lockd_gl(cmd, "ex", 0)) {
		/* Let pvremove -ff skip locks */
		if (pp.force == DONT_PROMPT_OVERRIDE)
			log_warn("WARNING: skipping global lock in lvmlockd for force.");
		else
			return_ECMD_FAILED;
	}
	cmd->lockd_gl_disable = 1;

	/* When forcibly clearing a PV we don't care about a VG lock. */
	if (pp.force == DONT_PROMPT_OVERRIDE)
		cmd->lockd_vg_disable = 1;

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	/*
	 * pvremove uses the same toollib function as pvcreate,
	 * but sets "is_remove" which changes the check function,
	 * and the actual create vs remove step.
	 */

	if (!pvcreate_each_device(cmd, handle, &pp))
		ret = ECMD_FAILED;
	else {
		/* pvcreate_each_device returns with orphans locked */
		unlock_vg(cmd, NULL, VG_ORPHANS);
		ret = ECMD_PROCESSED;
	}

	destroy_processing_handle(cmd, handle);
	return ret;
}
