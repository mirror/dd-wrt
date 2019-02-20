/**
 * @file support/nfsref/nfsref.h
 * @brief Declarations and definitions for nfsref command line tool
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

#ifndef UTILS_NFSREF_H
#define UTILS_NFSREF_H

/**
 * Junction types supported by the "nfsref" command
 */
enum nfsref_type {
	NFSREF_TYPE_UNSPECIFIED = 1,
	NFSREF_TYPE_NFS_BASIC,
	NFSREF_TYPE_NFS_FEDFS
};

int	 nfsref_add(enum nfsref_type type, const char *junct_path, char **argv,
				int optind);
int	 nfsref_remove(enum nfsref_type type, const char *junct_path);
int	 nfsref_lookup(enum nfsref_type type, const char *junct_path);

int	 nfsref_add_help(const char *progname);
int	 nfsref_remove_help(const char *progname);
int	 nfsref_lookup_help(const char *progname);

#endif	/* !UTILS_NFSREF_H */
