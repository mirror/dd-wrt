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

#include "lvm2cmdline.h"
#include "lib/label/label.h"
#include "lib/mm/memlock.h"

#include "tools/lvm2cmd.h"

#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/resource.h>

void *cmdlib_lvm2_init(unsigned static_compile)
{
	struct cmd_context *cmd;

	init_is_static(static_compile);
	if (!(cmd = init_lvm(1, 1)))
		return NULL;

	if (!lvm_register_commands(cmd, NULL))
		return NULL;

	return (void *) cmd;
}

int lvm2_run(void *handle, const char *cmdline)
{
	int argc, ret, oneoff = 0;
	char *args[MAX_ARGS], **argv, *cmdcopy = NULL;
	struct cmd_context *cmd;

	argv = args;

	if (!handle) {
		oneoff = 1;
		if (!(handle = lvm2_init())) {
			log_error("Handle initialisation failed.");
			return ECMD_FAILED;
		}
	}

	cmd = (struct cmd_context *) handle;

	cmd->argv = argv;

	if (!(cmdcopy = strdup(cmdline))) {
		log_error("Cmdline copy failed.");
		ret = ECMD_FAILED;
		goto out;
	}

	if (lvm_split(cmdcopy, &argc, argv, MAX_ARGS) == MAX_ARGS) {
		log_error("Too many arguments.  Limit is %d.", MAX_ARGS);
		ret = EINVALID_CMD_LINE;
		goto out;
	}

	if (!argc) {
		log_error("No command supplied");
		ret = EINVALID_CMD_LINE;
		goto out;
	}

	/* FIXME Temporary - move to libdevmapper */
	ret = ECMD_PROCESSED;
	if (!strcmp(cmdline, "_memlock_inc")) {
		memlock_inc_daemon(cmd);
	} else if (!strcmp(cmdline, "_memlock_dec"))
		memlock_dec_daemon(cmd);
	else if (!strcmp(cmdline, "_dmeventd_thin_command")) {
		if (setenv(cmdline, find_config_tree_str(cmd, dmeventd_thin_command_CFG, NULL), 1))
			ret = ECMD_FAILED;
	} else if (!strcmp(cmdline, "_dmeventd_vdo_command")) {
		if (setenv(cmdline, find_config_tree_str(cmd, dmeventd_vdo_command_CFG, NULL), 1))
			ret = ECMD_FAILED;
	} else
		ret = lvm_run_command(cmd, argc, argv);

      out:
	free(cmdcopy);

	if (oneoff)
		lvm2_exit(handle);

	return ret;
}

void lvm2_disable_dmeventd_monitoring(void *handle)
{
	init_run_by_dmeventd((struct cmd_context *) handle);
}

void lvm2_log_level(void *handle, int level)
{
	struct cmd_context *cmd = (struct cmd_context *) handle;

	cmd->default_settings.verbose = level - VERBOSE_BASE_LEVEL;
}

void lvm2_log_fn(lvm2_log_fn_t log_fn)
{
	init_log_fn(log_fn);
}

void lvm2_exit(void *handle)
{
	struct cmd_context *cmd = (struct cmd_context *) handle;

	lvm_fin(cmd);
}
