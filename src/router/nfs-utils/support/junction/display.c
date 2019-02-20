/**
 * @file support/junction/display.c
 * @brief Shared display helper functions
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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "junction.h"

/**
 * Return human-readable equivalent of a FedFsStatus value
 *
 * @param status FedFsStatus code
 * @return a static NUL-terminated C string
 */
const char *
nsdb_display_fedfsstatus(const FedFsStatus status)
{
	switch (status) {
	case FEDFS_OK:
		return "FEDFS_OK";
	case FEDFS_ERR_ACCESS:
		return "FEDFS_ERR_ACCESS";
	case FEDFS_ERR_BADCHAR:
		return "FEDFS_ERR_BADCHAR";
	case FEDFS_ERR_BADNAME:
		return "FEDFS_ERR_BADNAME";
	case FEDFS_ERR_NAMETOOLONG:
		return "FEDFS_ERR_NAMETOOLONG";
	case FEDFS_ERR_LOOP:
		return "FEDFS_ERR_LOOP";
	case FEDFS_ERR_BADXDR:
		return "FEDFS_ERR_BADXDR";
	case FEDFS_ERR_EXIST:
		return "FEDFS_ERR_EXIST";
	case FEDFS_ERR_INVAL:
		return "FEDFS_ERR_INVAL";
	case FEDFS_ERR_IO:
		return "FEDFS_ERR_IO";
	case FEDFS_ERR_NOSPC:
		return "FEDFS_ERR_NOSPC";
	case FEDFS_ERR_NOTJUNCT:
		return "FEDFS_ERR_NOTJUNCT";
	case FEDFS_ERR_NOTLOCAL:
		return "FEDFS_ERR_NOTLOCAL";
	case FEDFS_ERR_PERM:
		return "FEDFS_ERR_PERM";
	case FEDFS_ERR_ROFS:
		return "FEDFS_ERR_ROFS";
	case FEDFS_ERR_SVRFAULT:
		return "FEDFS_ERR_SVRFAULT";
	case FEDFS_ERR_NOTSUPP:
		return "FEDFS_ERR_NOTSUPP";
	case FEDFS_ERR_NSDB_ROUTE:
		return "FEDFS_ERR_NSDB_ROUTE";
	case FEDFS_ERR_NSDB_DOWN:
		return "FEDFS_ERR_NSDB_DOWN";
	case FEDFS_ERR_NSDB_CONN:
		return "FEDFS_ERR_NSDB_CONN";
	case FEDFS_ERR_NSDB_AUTH:
		return "FEDFS_ERR_NSDB_AUTH";
	case FEDFS_ERR_NSDB_LDAP:
		return "FEDFS_ERR_NSDB_LDAP";
	case FEDFS_ERR_NSDB_LDAP_VAL:
		return "FEDFS_ERR_NSDB_LDAP_VAL";
	case FEDFS_ERR_NSDB_NONCE:
		return "FEDFS_ERR_NSDB_NONCE";
	case FEDFS_ERR_NSDB_NOFSN:
		return "FEDFS_ERR_NSDB_NOFSN";
	case FEDFS_ERR_NSDB_NOFSL:
		return "FEDFS_ERR_NSDB_NOFSL";
	case FEDFS_ERR_NSDB_RESPONSE:
		return "FEDFS_ERR_NSDB_RESPONSE";
	case FEDFS_ERR_NSDB_FAULT:
		return "FEDFS_ERR_NSDB_FAULT";
	case FEDFS_ERR_NSDB_PARAMS:
		return "FEDFS_ERR_NSDB_PARAMS";
	case FEDFS_ERR_NSDB_LDAP_REFERRAL:
		return "FEDFS_ERR_NSDB_LDAP_REFERRAL";
	case FEDFS_ERR_NSDB_LDAP_REFERRAL_VAL:
		return "FEDFS_ERR_NSDB_LDAP_REFERRAL_VAL";
	case FEDFS_ERR_NSDB_PARAMS_LDAP_REFERRAL:
		return "FEDFS_ERR_NSDB_PARAMS_LDAP_REFERRAL";
	case FEDFS_ERR_PATH_TYPE_UNSUPP:
		return "FEDFS_ERR_PATH_TYPE_UNSUPP";
	case FEDFS_ERR_DELAY:
		return "FEDFS_ERR_DELAY";
	case FEDFS_ERR_NO_CACHE:
		return "FEDFS_ERR_NO_CACHE";
	case FEDFS_ERR_UNKNOWN_CACHE:
		return "FEDFS_ERR_UNKNOWN_CACHE";
	case FEDFS_ERR_NO_CACHE_UPDATE:
		return "FEDFS_ERR_NO_CACHE_UPDATE";
	default:
		break;
	}
	return "an unrecognized error code";
}

/**
 * Display human-readable FedFsStatus on stderr
 *
 * @param status FedFsStatus value to display
 */
void
nsdb_print_fedfsstatus(const FedFsStatus status)
{
	if (status == FEDFS_OK) {
		printf("Call completed successfully\n");
		return;
	}

	fprintf(stderr, "Server returned %s\n",
			nsdb_display_fedfsstatus(status));
}
