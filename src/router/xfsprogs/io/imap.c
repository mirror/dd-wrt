// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2001-2003,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"
#include "libfrog/logging.h"
#include "libfrog/fsgeom.h"
#include "libfrog/bulkstat.h"

static cmdinfo_t imap_cmd;

static int
imap_f(int argc, char **argv)
{
	struct xfs_fd		xfd = XFS_FD_INIT(file->fd);
	struct xfs_inumbers_req	*ireq;
	uint32_t		nent;
	int			i;
	int			error;

	if (argc != 2)
		nent = 1;
	else
		nent = atoi(argv[1]);

	error = -xfrog_inumbers_alloc_req(nent, 0, &ireq);
	if (error) {
		xfrog_perror(error, "alloc req");
		exitcode = 1;
		return 0;
	}

	while ((error = -xfrog_inumbers(&xfd, ireq)) == 0 &&
	       ireq->hdr.ocount > 0) {
		for (i = 0; i < ireq->hdr.ocount; i++) {
			printf(_("ino %10"PRIu64" count %2d mask %016"PRIx64"\n"),
				ireq->inumbers[i].xi_startino,
				ireq->inumbers[i].xi_alloccount,
				ireq->inumbers[i].xi_allocmask);
		}
	}

	if (error) {
		xfrog_perror(error, "xfsctl(XFS_IOC_FSINUMBERS)");
		exitcode = 1;
	}
	free(ireq);
	return 0;
}

void
imap_init(void)
{
	imap_cmd.name = "imap";
	imap_cmd.cfunc = imap_f;
	imap_cmd.argmin = 0;
	imap_cmd.argmax = 1;
	imap_cmd.args = _("[nentries]");
	imap_cmd.flags = CMD_NOMAP_OK | CMD_FLAG_ONESHOT;
	imap_cmd.oneline = _("inode map for filesystem of current file");

	if (expert)
		add_command(&imap_cmd);
}
