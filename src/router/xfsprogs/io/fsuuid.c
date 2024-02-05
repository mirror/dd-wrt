// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 Oracle.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "command.h"
#include "init.h"
#include "io.h"
#include "libfrog/fsgeom.h"
#include "libfrog/logging.h"

static cmdinfo_t fsuuid_cmd;

static int
fsuuid_f(
	int			argc,
	char			**argv)
{
	struct xfs_fsop_geom	fsgeo;
	int			ret;
	char			bp[40];

	ret = -xfrog_geometry(file->fd, &fsgeo);

	if (ret) {
		xfrog_perror(ret, "XFS_IOC_FSGEOMETRY");
		exitcode = 1;
	} else {
		platform_uuid_unparse((uuid_t *)fsgeo.uuid, bp);
		printf("UUID = %s\n", bp);
	}

	return 0;
}

void
fsuuid_init(void)
{
	fsuuid_cmd.name = "fsuuid";
	fsuuid_cmd.cfunc = fsuuid_f;
	fsuuid_cmd.argmin = 0;
	fsuuid_cmd.argmax = 0;
	fsuuid_cmd.flags = CMD_FLAG_ONESHOT | CMD_NOMAP_OK;
	fsuuid_cmd.oneline = _("get mounted filesystem UUID");

	add_command(&fsuuid_cmd);
}
