/**
 * @file utils/nfsref/add.c
 * @brief Add junction metadata to a local file system object
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

#include <sys/stat.h>
#include <sys/types.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <uuid/uuid.h>

#include "junction.h"
#include "xlog.h"
#include "nfsref.h"

/**
 * Default cache expiration for FSN information
 */
#define FSN_DEFAULT_TTL		(300)

/**
 * Display help message for "add" subcommand
 *
 * @param progname NUL-terminated C string containing name of program
 * @return program exit status
 */
int
nfsref_add_help(const char *progname)
{
	fprintf(stderr, " \n");

	fprintf(stderr, "Usage: %s [ -t type ] add <junction path> "
			"<server> <export> [ <server> <export> ... ]\n\n",
		progname);

	fprintf(stderr, "Add a new junction containing the specified list "
			"of fileset locations.\n");
	fprintf(stderr, "<junction path> is the filename of the new junction.  "
			"<server> is the hostname\n");
	fprintf(stderr, "or IP address of an NFS server where the fileset is "
			"located.  <export> is the\n");
	fprintf(stderr, "export pathname of the fileset on that server.\n\n");

	fprintf(stderr, "For NFS basic junctions, the location list is stored "
			"locally in the junction.\n");
	fprintf(stderr, "For FedFS junctions, the location list is stored "
			"as new FSN and FSL records\n");
	fprintf(stderr, "on an NSDB.\n");

	return EXIT_SUCCESS;
}

/**
 * Fill in default settings for NFSv4.0 fs_locations4
 *
 * @param new NFS location structure to fill in
 *
 * See section 5.1.3.2 of the NSDB protocol draft.
 */
static void
nfsref_add_fsloc_defaults(struct nfs_fsloc *new)
{
	new->nfl_hostport = 0;
	new->nfl_flags.nfl_varsub = false;
	new->nfl_currency = -1;
	new->nfl_validfor = 0;
	new->nfl_genflags.nfl_writable = false;
	new->nfl_genflags.nfl_going = false;
	new->nfl_genflags.nfl_split = true;
	new->nfl_transflags.nfl_rdma = true;
	new->nfl_info.nfl_simul = 0;
	new->nfl_info.nfl_handle = 0;
	new->nfl_info.nfl_fileid = 0;
	new->nfl_info.nfl_writever = 0;
	new->nfl_info.nfl_change = 0;
	new->nfl_info.nfl_readdir = 0;
	new->nfl_info.nfl_readrank = 0;
	new->nfl_info.nfl_readorder = 0;
	new->nfl_info.nfl_writerank = 0;
	new->nfl_info.nfl_writeorder = 0;
}

/**
 * Convert a pair of command line arguments to one nfs_fsloc structure
 *
 * @param server NUL-terminated C string containing file server hostname
 * @param rootpath NUL-terminated C string containing POSIX-style export path
 * @param fsloc OUT: NFS location structure
 * @return a FedFsStatus code
 *
 * If nfsref_add_build_fsloc() returns FEDFS_OK, caller must free the
 * returned fsloc with nfs_free_location().
 */
static FedFsStatus
nfsref_add_build_fsloc(const char *server, const char *rootpath,
		struct nfs_fsloc **fsloc)
{
	struct nfs_fsloc *new;
	FedFsStatus retval;

	if (server == NULL || rootpath == NULL)
		return FEDFS_ERR_INVAL;

	xlog(D_GENERAL, "%s: Building fsloc for %s:%s",
		__func__, server, rootpath);

	new = nfs_new_location();
	if (new == NULL) {
		xlog(D_GENERAL, "%s: No memory", __func__);
		return FEDFS_ERR_SVRFAULT;
	}

	new->nfl_hostname = strdup(server);
	if (new->nfl_hostname == NULL) {
		nfs_free_location(new);
		xlog(D_GENERAL, "%s: No memory", __func__);
		return FEDFS_ERR_SVRFAULT;
	}

	retval = nsdb_posix_to_path_array(rootpath, &new->nfl_rootpath);
	if (retval != FEDFS_OK) {
		nfs_free_location(new);
		return retval;
	}

	nfsref_add_fsloc_defaults(new);
	*fsloc = new;
	return FEDFS_OK;
}

/**
 * Convert array of command line arguments to list of nfs_fsloc structures
 *
 * @param argv array of pointers to NUL-terminated C strings contains arguments
 * @param optind index of "argv" where "add" subcommand arguments start
 * @param fslocs OUT: list of NFS locations
 * @return a FedFsStatus code
 *
 * If nfsref_add_build_fsloc_list() returns FEDFS_OK, caller must free the
 * returned list of fslocs with nfs_free_locations().
 */
static FedFsStatus
nfsref_add_build_fsloc_list(char **argv, int optind, struct nfs_fsloc **fslocs)
{
	struct nfs_fsloc *fsloc, *result = NULL;
	FedFsStatus retval;
	int i;

	for (i = optind + 2; argv[i] != NULL; i += 2) {
		retval = nfsref_add_build_fsloc(argv[i], argv[i + 1], &fsloc);
		if (retval != FEDFS_OK) {
			nfs_free_locations(result);
			return retval;
		}
		if (result == NULL)
			result = fsloc;
		else
			result->nfl_next = fsloc;
	}
	if (result == NULL)
		return FEDFS_ERR_INVAL;

	*fslocs = result;
	return FEDFS_OK;
}

/**
 * Add NFS locations to a junction
 *
 * @param junct_path NUL-terminated C string containing pathname of junction
 * @param argv array of pointers to NUL-terminated C strings contains arguments
 * @param optind index of "argv" where "add" subcommand arguments start
 * @return program exit status
 */
static int
nfsref_add_nfs_basic(const char *junct_path, char **argv, int optind)
{
	struct nfs_fsloc *fslocs = NULL;
	FedFsStatus retval;

	xlog(D_GENERAL, "%s: Adding basic junction to %s",
		__func__, junct_path);

	retval = nfsref_add_build_fsloc_list(argv, optind, &fslocs);
	switch (retval) {
	case FEDFS_OK:
		break;
	case FEDFS_ERR_INVAL:
		xlog(L_ERROR, "Missing arguments");
		return EXIT_FAILURE;
	case FEDFS_ERR_SVRFAULT:
		xlog(L_ERROR, "No memory");
		return EXIT_FAILURE;
	default:
		xlog(L_ERROR, "Failed to add NFS location metadata to %s: %s",
			junct_path, nsdb_display_fedfsstatus(retval));
		return EXIT_FAILURE;
	}

	retval = nfs_add_junction(junct_path, fslocs);
	nfs_free_locations(fslocs);
	switch (retval) {
	case FEDFS_OK:
		break;
	case FEDFS_ERR_EXIST:
		xlog(L_ERROR, "%s already contains junction metadata",
			junct_path);
		return EXIT_FAILURE;
	default:
		xlog(L_ERROR, "Failed to add NFS location metadata to %s: %s",
			junct_path, nsdb_display_fedfsstatus(retval));
		return EXIT_FAILURE;
	}

	printf("Created junction %s\n", junct_path);
	return EXIT_SUCCESS;
}

/**
 * Add locations to a junction
 *
 * @param type type of junction to add
 * @param junct_path NUL-terminated C string containing pathname of junction
 * @param argv array of pointers to NUL-terminated C strings contains arguments
 * @param optind index of "argv" where "add" subcommand arguments start
 * @return program exit status
 */
int
nfsref_add(enum nfsref_type type, const char *junct_path, char **argv, int optind)
{
	if (mkdir(junct_path, 0755) == -1)
		if (errno != EEXIST) {
			xlog(L_ERROR, "Failed to create junction object: %m");
			return EXIT_FAILURE;
		}

	switch (type) {
	case NFSREF_TYPE_UNSPECIFIED:
	case NFSREF_TYPE_NFS_BASIC:
		return nfsref_add_nfs_basic(junct_path, argv, optind);
	default:
		xlog(L_ERROR, "Unrecognized junction type");
	}
	return EXIT_FAILURE;
}
