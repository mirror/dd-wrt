/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
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

#include "tools.h"

static char *_expand_filename(const char *template, const char *vg_name,
			      char **last_filename)
{
	char *filename;

	if (security_level()) {
		if (!(filename = strdup(template))) {
			log_error("Failed to allocate filename.");
			return NULL;
		}
		goto out;
	}

	if (!(filename = malloc(PATH_MAX))) {
		log_error("Failed to allocate filename.");
		return NULL;
	}

	if (dm_snprintf(filename, PATH_MAX, template, vg_name) < 0) {
		log_error("Error processing filename template %s",
			   template);
		free(filename);
		return NULL;
	}
	if (*last_filename && !strncmp(*last_filename, filename, PATH_MAX)) {
		log_error("VGs must be backed up into different files. "
			  "Use %%s in filename for VG name.");
		free(filename);
		return NULL;
	}
out:
	free(*last_filename);
	*last_filename = filename;

	return filename;
}

static int _vg_backup_single(struct cmd_context *cmd, const char *vg_name,
			     struct volume_group *vg,
			     struct processing_handle *handle)
{
	char **last_filename = (char **)handle->custom_handle;
	char *filename;

	if (arg_is_set(cmd, file_ARG)) {
		if (!(filename = _expand_filename(arg_value(cmd, file_ARG),
						  vg->name, last_filename)))
			return_ECMD_FAILED;

		if (!backup_to_file(filename, vg->cmd->cmd_line, vg))
			return_ECMD_FAILED;
	} else {
		if (vg_read_error(vg) == FAILED_INCONSISTENT) {
			log_error("No backup taken: specify filename with -f "
				  "to backup an inconsistent VG");
			return ECMD_FAILED;
		}

		/* just use the normal backup code */
		backup_enable(cmd, 1);	/* force a backup */
		if (!backup(vg))
			return_ECMD_FAILED;
	}

	log_print_unless_silent("Volume group \"%s\" successfully backed up.", vg_name);

	return ECMD_PROCESSED;
}

int vgcfgbackup(struct cmd_context *cmd, int argc, char **argv)
{
	int ret;
	char *last_filename = NULL;
	struct processing_handle *handle = NULL;

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &last_filename;

	init_pvmove(1);

	ret = process_each_vg(cmd, argc, argv, NULL, NULL, READ_ALLOW_INCONSISTENT, 0,
			      handle, &_vg_backup_single);

	free(last_filename);

	init_pvmove(0);

	destroy_processing_handle(cmd, handle);
	return ret;
}
