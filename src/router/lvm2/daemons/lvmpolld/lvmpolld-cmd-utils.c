/*
 * Copyright (C) 2015 Red Hat, Inc.
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

#include "lvmpolld-common.h"

/* extract this info from autoconf/automake files */
#define LVPOLL_CMD "lvpoll"

#define MIN_ARGV_SIZE  8

static const char *const polling_ops[] = {
	[PVMOVE] = LVMPD_REQ_PVMOVE,
	[CONVERT] = LVMPD_REQ_CONVERT,
	[MERGE] = LVMPD_REQ_MERGE,
	[MERGE_THIN] = LVMPD_REQ_MERGE_THIN
};

const char *polling_op(enum poll_type type)
{
	return type < POLL_TYPE_MAX ? polling_ops[type] : "<undefined>";
}

static int add_to_cmd_arr(const char ***cmdargv, const char *str, unsigned *ind)
{
	const char **newargv;

	if (*ind && !(*ind % MIN_ARGV_SIZE)) {
		newargv = realloc(*cmdargv, (*ind / MIN_ARGV_SIZE + 1) * MIN_ARGV_SIZE * sizeof(char *));
		if (!newargv)
			return 0;
		*cmdargv = newargv;
	}

	*(*cmdargv + (*ind)++) = str;

	return 1;
}

const char **cmdargv_ctr(const struct lvmpolld_lv *pdlv, const char *lvm_binary, unsigned abort_polling, unsigned handle_missing_pvs)
{
	unsigned i = 0;
	const char **cmd_argv = malloc(MIN_ARGV_SIZE * sizeof(char *));

	if (!cmd_argv)
		return NULL;

	/* path to lvm2 binary */
	if (!add_to_cmd_arr(&cmd_argv, lvm_binary, &i))
		goto err;

	/* cmd to execute */
	if (!add_to_cmd_arr(&cmd_argv, LVPOLL_CMD, &i))
		goto err;

	/* transfer internal polling interval */
	if (pdlv->sinterval &&
	    (!add_to_cmd_arr(&cmd_argv, "--interval", &i) ||
	     !add_to_cmd_arr(&cmd_argv, pdlv->sinterval, &i)))
		goto err;

	/* pass abort param */
	if (abort_polling &&
	    !add_to_cmd_arr(&cmd_argv, "--abort", &i))
		goto err;

	/* pass handle-missing-pvs. used by mirror polling operation */
	if (handle_missing_pvs &&
	    !add_to_cmd_arr(&cmd_argv, "--handlemissingpvs", &i))
		goto err;

	/* one of: "convert", "pvmove", "merge", "merge_thin" */
	if (!add_to_cmd_arr(&cmd_argv, "--polloperation", &i) ||
	    !add_to_cmd_arr(&cmd_argv, polling_ops[pdlv->type], &i))
		goto err;

	/* vg/lv name */
	if (!add_to_cmd_arr(&cmd_argv, pdlv->lvname, &i))
		goto err;

	/* disable metadata backup */
	if (!add_to_cmd_arr(&cmd_argv, "-An", &i))
		goto err;

	/* terminating NULL */
	if (!add_to_cmd_arr(&cmd_argv, NULL, &i))
		goto err;

	return cmd_argv;
err:
	free(cmd_argv);
	return NULL;
}

/* FIXME: in fact exclude should be va list */
static int copy_env(const char ***cmd_envp, unsigned *i, const char *exclude)
{
	const char * const* tmp = (const char * const*) environ;

	if (!tmp)
		return 0;

	while (*tmp) {
		if (strncmp(*tmp, exclude, strlen(exclude)) && !add_to_cmd_arr(cmd_envp, *tmp, i))
			return 0;
		tmp++;
	}

	return 1;
}

const char **cmdenvp_ctr(const struct lvmpolld_lv *pdlv)
{
	unsigned i = 0;
	const char **cmd_envp = malloc(MIN_ARGV_SIZE * sizeof(char *));

	if (!cmd_envp)
		return NULL;

	/* copy whole environment from lvmpolld, exclude LVM_SYSTEM_DIR if set */
	if (!copy_env(&cmd_envp, &i, "LVM_SYSTEM_DIR="))
		goto err;

	/* Add per client LVM_SYSTEM_DIR variable if set */
	if (*pdlv->lvm_system_dir_env && !add_to_cmd_arr(&cmd_envp, pdlv->lvm_system_dir_env, &i))
		goto err;

	/* terminating NULL */
	if (!add_to_cmd_arr(&cmd_envp, NULL, &i))
		goto err;

	return cmd_envp;
err:
	free(cmd_envp);
	return NULL;
}
