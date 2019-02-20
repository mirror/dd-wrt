/*
 * Copyright (C) 2002-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2006 Red Hat, Inc. All rights reserved.
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

#include "lib/misc/lib.h"
#include "sharedlib.h"
#include "lib/config/config.h"
#include "lib/commands/toolcontext.h"

#include <limits.h>
#include <sys/stat.h>
#include <dlfcn.h>

void get_shared_library_path(struct cmd_context *cmd, const char *libname,
			     char *path, size_t path_len)
{
	struct stat info;

	if (!path_len)
		return;

	/* If libname doesn't begin with '/' then use lib_dir/libname,
	 * if present */
	if (libname[0] == '/' ||
	    (!cmd->lib_dir &&
	     !(cmd->lib_dir = find_config_tree_str(cmd, global_library_dir_CFG, NULL))) ||
	    (dm_snprintf(path, path_len, "%s/%s", cmd->lib_dir,
			 libname) == -1) || stat(path, &info) == -1) {
		(void) dm_strncpy(path, libname, path_len);
	}
}
