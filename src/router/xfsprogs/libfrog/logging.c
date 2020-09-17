// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include <errno.h>
#include <stdio.h>
#include "logging.h"

/* Print an error. */
void
xfrog_perror(
	int		error,
	const char	*s)
{
	errno = error < 0 ? -error : error;
	perror(s);
}
