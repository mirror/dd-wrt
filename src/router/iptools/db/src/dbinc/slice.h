/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
 */
/*
 * $Id$
 */
#ifndef	_DB_SLICE_H_
#define	_DB_SLICE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef HAVE_SLICES

#define	DB_SLICE_PREFIX	"__db.slice"
#define	DB_MAX_SLICES	1000	/* 0 -> 999 */

/*
 * Default name for a sliced sub-environment's env->db_home. Use its sizeof()
 * when calculating the space needed for allocating a string, as for snprintf()
 * This is enough for up to 999 slices if DB_SLICE_DIGITS is 3.
 */
#define	DB_SLICE_SUBDIR_FMT	"%03u"
#define	DB_SLICE_SUBDIR		(DB_SLICE_PREFIX DB_SLICE_SUBDIR_FMT)

/*
 * The containing database has a small number of metadata records.
 * V1:
 *	version			snprintf(DB_SLICE_METADATA_VERSION)
 *	count			snprintf(DB_ENV.slice_cnt)
 *	fileid#%03d		DB.get_fileid() for each slice's portion.
 *
 */
/* The initial release of slices has this metadata version. */
#define	DB_SLICE_METADATA_VERSION	1

#define	DB_SLICE_METADATA_FILEID_FMT	"fileid#" DB_SLICE_SUBDIR_FMT

#endif

#if defined(__cplusplus)
}
#endif

#endif
