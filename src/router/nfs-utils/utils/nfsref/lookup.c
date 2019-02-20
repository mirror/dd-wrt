/**
 * @file utils/nfsref/lookup.c
 * @brief Examine junction metadata from a local file system object
 */

/*
 * Copyright 2011, 2018 Oracle.  All rights reserved.
 *
 * This file is part of nfs-utils.
 *
 * nfs-utils is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * nfs-utils is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2.0 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2.0 along with nfs-utils.  If not, see:
 *
 *	http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <rpcsvc/nfs_prot.h>

#include "junction.h"
#include "xlog.h"
#include "nfsref.h"

/**
 * Display help message for "lookup" subcommand
 *
 * @param progname NUL-terminated C string containing name of program
 * @return program exit status
 */
int
nfsref_lookup_help(const char *progname)
{
	fprintf(stderr, " \n");

	fprintf(stderr, "Usage: %s [ -t type ] lookup <junction path>\n\n",
		progname);

	fprintf(stderr, "Display the contents of the junction at "
			"<junction path>.  For NFS basic\n");
	fprintf(stderr, "junctions, the local contents of the junction "
			"are displayed.  For FedFS\n");
	fprintf(stderr, "junctions, FSL records are retrieved from the "
			"NSDB and displayed.\n");

	return EXIT_SUCCESS;
}

/**
 * Convert a boolean value into a displayable string constant
 *
 * @param value boolean value
 * @return NUL-terminated static constant C string
 */
static const char *
nfsref_lookup_display_boolean(_Bool value)
{
	return value ? "true" : "false";
}

/**
 * Display a single NFS location
 *
 * @param fsloc pointer to an NFS location structure
 */
static void
nfsref_lookup_display_nfs_location(struct nfs_fsloc *fsloc)
{
	char *rootpath;

	if (nsdb_path_array_to_posix(fsloc->nfl_rootpath, &rootpath) == FEDFS_OK) {
		printf("%s:%s\n", fsloc->nfl_hostname, rootpath);
		free(rootpath);
	} else
		printf("%s: - Invalid root path -\n", fsloc->nfl_hostname);
	printf("\n");

	printf("\tNFS port:\t%u\n", fsloc->nfl_hostport);
	printf("\tValid for:\t%d\n", fsloc->nfl_validfor);
	printf("\tCurrency:\t%d\n", fsloc->nfl_currency);
	printf("\tFlags:\t\tvarsub(%s)\n",
		nfsref_lookup_display_boolean(fsloc->nfl_flags.nfl_varsub));

	printf("\tGenFlags:\twritable(%s), going(%s), split(%s)\n",
		nfsref_lookup_display_boolean(fsloc->nfl_genflags.nfl_writable),
		nfsref_lookup_display_boolean(fsloc->nfl_genflags.nfl_going),
		nfsref_lookup_display_boolean(fsloc->nfl_genflags.nfl_split));
	printf("\tTransFlags:\trdma(%s)\n",
		nfsref_lookup_display_boolean(fsloc->nfl_transflags.nfl_rdma));

	printf("\tClass:\t\tsimul(%u), handle(%u), fileid(%u)\n",
		fsloc->nfl_info.nfl_simul,
		fsloc->nfl_info.nfl_handle,
		fsloc->nfl_info.nfl_fileid);
	printf("\tClass:\t\twritever(%u), change(%u), readdir(%u)\n",
		fsloc->nfl_info.nfl_writever,
		fsloc->nfl_info.nfl_change,
		fsloc->nfl_info.nfl_readdir);
	printf("\tRead:\t\trank(%u), order(%u)\n",
		fsloc->nfl_info.nfl_readrank, fsloc->nfl_info.nfl_readorder);
	printf("\tWrite:\t\trank(%u), order(%u)\n",
		fsloc->nfl_info.nfl_writerank, fsloc->nfl_info.nfl_writeorder);

	printf("\n");
}

/**
 * Display a list of NFS locations
 *
 * @param fslocs list of NFS locations to display
 */
static void
nfsref_lookup_display_nfs_locations(struct nfs_fsloc *fslocs)
{
	struct nfs_fsloc *fsloc;

	for (fsloc = fslocs; fsloc != NULL; fsloc = fsloc->nfl_next)
		nfsref_lookup_display_nfs_location(fsloc);
}

/**
 * List NFS locations in an nfs-basic junction
 *
 * @param junct_path NUL-terminated C string containing pathname of junction
 * @return program exit status
 */
static int
nfsref_lookup_nfs_basic(const char *junct_path)
{
	struct nfs_fsloc *fslocs = NULL;
	FedFsStatus retval;

	xlog(D_GENERAL, "%s: Looking up basic junction in %s",
		__func__, junct_path);

	retval = nfs_is_junction(junct_path);
	switch (retval) {
	case FEDFS_OK:
		break;
	case FEDFS_ERR_NOTJUNCT:
		xlog(L_ERROR, "%s is not an nfs-basic junction", junct_path);
		return EXIT_FAILURE;
	default:
		xlog(L_ERROR, "Failed to access %s: %s",
			junct_path, nsdb_display_fedfsstatus(retval));
		return EXIT_FAILURE;
	}

	retval = nfs_get_locations(junct_path, &fslocs);
	if (retval != FEDFS_OK) {
		xlog(L_ERROR, "Failed to access %s: %s",
			junct_path, nsdb_display_fedfsstatus(retval));
		return EXIT_FAILURE;
	}

	nfsref_lookup_display_nfs_locations(fslocs);

	nfs_free_locations(fslocs);
	return EXIT_SUCCESS;
}

/**
 * Resolve either a FedFS or NFS basic junction
 *
 * @param junct_path NUL-terminated C string containing pathname of junction
 * @return program exit status
 */
static int
nfsref_lookup_unspecified(const char *junct_path)
{
	FedFsStatus retval;

	retval = nfs_is_junction(junct_path);
	if (retval == FEDFS_OK)
		return nfsref_lookup_nfs_basic(junct_path);
	xlog(L_ERROR, "%s is not a junction", junct_path);
	return EXIT_FAILURE;
}

/**
 * Enumerate metadata of a junction
 *
 * @param type type of junction to add
 * @param junct_path NUL-terminated C string containing pathname of junction
 * @return program exit status
 */
int
nfsref_lookup(enum nfsref_type type, const char *junct_path)
{
	switch (type) {
	case NFSREF_TYPE_UNSPECIFIED:
		return nfsref_lookup_unspecified(junct_path);
	case NFSREF_TYPE_NFS_BASIC:
		return nfsref_lookup_nfs_basic(junct_path);
	default:
		xlog(L_ERROR, "Unrecognized junction type");
	}
	return EXIT_FAILURE;
}
