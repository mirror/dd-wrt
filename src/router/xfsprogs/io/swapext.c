// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Red Hat, Inc.
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"
#include "libfrog/logging.h"
#include "libfrog/fsgeom.h"
#include "libfrog/bulkstat.h"

static cmdinfo_t swapext_cmd;

static void
swapext_help(void)
{
	printf(_(
"\n"
" Swaps extents between the open file descriptor and the supplied filename.\n"
"\n"));
}

static int
swapext_f(
	int			argc,
	char			**argv)
{
	struct xfs_fd		fxfd = XFS_FD_INIT(file->fd);
	struct xfs_bulkstat	bulkstat;
	int			fd;
	int			error;
	struct xfs_swapext	sx;
	struct stat		stat;

	/* open the donor file */
	fd = openfile(argv[1], NULL, 0, 0, NULL);
	if (fd < 0)
		return 0;

	/*
	 * stat the target file to get the inode number and use the latter to
	 * get the bulkstat info for the swapext cmd.
	 */
	error = fstat(file->fd, &stat);
	if (error) {
		perror("fstat");
		goto out;
	}

	error = -xfrog_bulkstat_single(&fxfd, stat.st_ino, 0, &bulkstat);
	if (error) {
		xfrog_perror(error, "bulkstat");
		goto out;
	}
	error = -xfrog_bulkstat_v5_to_v1(&fxfd, &sx.sx_stat, &bulkstat);
	if (error) {
		xfrog_perror(error, "bulkstat conversion");
		goto out;
	}
	sx.sx_version = XFS_SX_VERSION;
	sx.sx_fdtarget = file->fd;
	sx.sx_fdtmp = fd;
	sx.sx_offset = 0;
	sx.sx_length = stat.st_size;
	error = ioctl(file->fd, XFS_IOC_SWAPEXT, &sx);
	if (error)
		perror("swapext");

out:
	close(fd);
	return 0;
}

void
swapext_init(void)
{
	swapext_cmd.name = "swapext";
	swapext_cmd.cfunc = swapext_f;
	swapext_cmd.argmin = 1;
	swapext_cmd.argmax = 1;
	swapext_cmd.flags = CMD_NOMAP_OK;
	swapext_cmd.args = _("<donorfile>");
	swapext_cmd.oneline = _("Swap extents between files.");
	swapext_cmd.help = swapext_help;

	add_command(&swapext_cmd);
}
