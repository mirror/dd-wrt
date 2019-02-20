/*
 * @file support/include/junction.h
 * @brief Declarations for libjunction.a
 */

/*
 * Copyright 2010, 2018 Oracle.  All rights reserved.
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

#ifndef _NFS_JUNCTION_H_
#define _NFS_JUNCTION_H_

#include <stdint.h>

/*
 * The libjunction APIs use the status codes from the FedFS ADMIN
 * protocol, which includes non-errno codes like FEDFS_ERR_NOTJUNCT.
 */
enum FedFsStatus {
	FEDFS_OK = 0,
	FEDFS_ERR_ACCESS = 1,
	FEDFS_ERR_BADCHAR = 2,
	FEDFS_ERR_BADNAME = 3,
	FEDFS_ERR_NAMETOOLONG = 4,
	FEDFS_ERR_LOOP = 5,
	FEDFS_ERR_BADXDR = 6,
	FEDFS_ERR_EXIST = 7,
	FEDFS_ERR_INVAL = 8,
	FEDFS_ERR_IO = 9,
	FEDFS_ERR_NOSPC = 10,
	FEDFS_ERR_NOTJUNCT = 11,
	FEDFS_ERR_NOTLOCAL = 12,
	FEDFS_ERR_PERM = 13,
	FEDFS_ERR_ROFS = 14,
	FEDFS_ERR_SVRFAULT = 15,
	FEDFS_ERR_NOTSUPP = 16,
	FEDFS_ERR_NSDB_ROUTE = 17,
	FEDFS_ERR_NSDB_DOWN = 18,
	FEDFS_ERR_NSDB_CONN = 19,
	FEDFS_ERR_NSDB_AUTH = 20,
	FEDFS_ERR_NSDB_LDAP = 21,
	FEDFS_ERR_NSDB_LDAP_VAL = 22,
	FEDFS_ERR_NSDB_NONCE = 23,
	FEDFS_ERR_NSDB_NOFSN = 24,
	FEDFS_ERR_NSDB_NOFSL = 25,
	FEDFS_ERR_NSDB_RESPONSE = 26,
	FEDFS_ERR_NSDB_FAULT = 27,
	FEDFS_ERR_NSDB_PARAMS = 28,
	FEDFS_ERR_NSDB_LDAP_REFERRAL = 29,
	FEDFS_ERR_NSDB_LDAP_REFERRAL_VAL = 30,
	FEDFS_ERR_NSDB_LDAP_REFERRAL_NOTFOLLOWED = 31,
	FEDFS_ERR_NSDB_PARAMS_LDAP_REFERRAL = 32,
	FEDFS_ERR_PATH_TYPE_UNSUPP = 33,
	FEDFS_ERR_DELAY = 34,
	FEDFS_ERR_NO_CACHE = 35,
	FEDFS_ERR_UNKNOWN_CACHE = 36,
	FEDFS_ERR_NO_CACHE_UPDATE = 37,
};
typedef enum FedFsStatus FedFsStatus;

/**
 * Contains NFS fileset location information
 *
 * Each of these represents one server:/rootpath pair.  The NFS
 * implementation can coalesce multiple pairs into a single
 * fs_location4 result if jfl_rootpath is the same across
 * multiple servers.
 *
 * The nfl_server field can contain either one presentation format
 * IP address or one DNS hostname.
 *
 * See Section 11.9 and 11.10 of RFC 5661 or section 4.2.2.3 and
 * 4.2.2.4 of the NSDB protocol draft for details.
 */

struct nfs_fsloc {
	struct nfs_fsloc	 *nfl_next;

	char			 *nfl_hostname;
	uint16_t		  nfl_hostport;
	char			**nfl_rootpath;

	struct {
		_Bool		  nfl_varsub;
	} nfl_flags;
	int32_t			  nfl_currency;
	int32_t			  nfl_validfor;

	struct {
		_Bool		  nfl_writable, nfl_going, nfl_split;
	} nfl_genflags;
	struct {
		_Bool		  nfl_rdma;
	} nfl_transflags;
	struct {
		uint8_t		  nfl_simul, nfl_handle, nfl_fileid;
		uint8_t		  nfl_writever, nfl_change, nfl_readdir;
		uint8_t		  nfl_readrank, nfl_writerank;
		uint8_t		  nfl_readorder, nfl_writeorder;
	} nfl_info;
};


/**
 ** NFS location data management functions
 **/

void		  nfs_free_location(struct nfs_fsloc *location);
void		  nfs_free_locations(struct nfs_fsloc *locations);
struct nfs_fsloc *nfs_new_location(void);

__attribute_malloc__
char		**nfs_dup_string_array(char **array);
void		  nfs_free_string_array(char **array);


/**
 ** NFS junction management functions
 **/

FedFsStatus	 nfs_delete_junction(const char *pathname);
FedFsStatus	 nfs_add_junction(const char *pathname,
				struct nfs_fsloc *locations);
FedFsStatus	 nfs_get_locations(const char *pathname,
				struct nfs_fsloc **locations);
FedFsStatus	 nfs_is_prejunction(const char *pathname);
FedFsStatus	 nfs_is_junction(const char *pathname);


/**
 ** Flush kernel NFS server's export cache
 **/
FedFsStatus	 junction_flush_exports_cache(void);

/**
 ** Pathname conversion helpers
 **/
void		 nsdb_free_string_array(char **strings);
FedFsStatus	 nsdb_path_array_to_posix(char * const *path_array,
				char **pathname);
FedFsStatus	 nsdb_posix_to_path_array(const char *pathname,
				char ***path_array);

/**
 ** Readability helpers
 **/

const char      *nsdb_display_fedfsstatus(const FedFsStatus status);
void             nsdb_print_fedfsstatus(const FedFsStatus status);

#endif	/* !_NFS_JUNCTION_H_ */
