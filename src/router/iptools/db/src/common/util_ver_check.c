/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"

/*
 * __db_util_version_check --
 *	Check if the right version of the DB library is loaded.
 *
 * PUBLIC: int __db_util_version_check __P((const char *));
 */
int
__db_util_version_check(progname)
	const char *progname;
{
	int v_major, v_minor, v_patch;

	/* Make sure we're loaded with the right version of the DB library. */
	(void)db_version(&v_major, &v_minor, &v_patch);
	if (v_major != DB_VERSION_MAJOR || v_minor != DB_VERSION_MINOR) {
		fprintf(stderr, DB_STR_A("5003",
		    "%s: version %d.%d doesn't match library version %d.%d\n",
		    "%s %d %d %d %d\n"), progname, DB_VERSION_MAJOR,
		    DB_VERSION_MINOR, v_major, v_minor);
		return (DB_VERSION_MISMATCH);
	}
	return (0);
}
