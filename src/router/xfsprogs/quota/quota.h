// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef XFS_QUOTA_QUOTA_H_
#define XFS_QUOTA_QUOTA_H_

#include "xqm.h"
#include "libfrog/paths.h"
#include "libfrog/projects.h"
#include <stdbool.h>

/*
 * Different forms of XFS quota
 */
enum {
	XFS_BLOCK_QUOTA	=	0x1,
	XFS_INODE_QUOTA =	0x2,
	XFS_RTBLOCK_QUOTA =	0x4,
};

/*
 * System call definitions mapping to platform-specific quotactl
 */
extern int xfsquotactl(int __cmd, const char *__device,
			uint __type, uint __id, void * __addr);
enum {
	XFS_QUOTAON,	/* enable accounting/enforcement */
	XFS_QUOTAOFF,	/* disable accounting/enforcement */
	XFS_GETQUOTA,	/* get disk limits and usage */
	XFS_SETQLIM,	/* set disk limits */
	XFS_GETQSTAT,	/* get quota subsystem status */
	XFS_QUOTARM,	/* free disk space used by dquots */
	XFS_QSYNC,	/* flush delayed allocate space */
	XFS_GETQSTATV,	/* newer version of quota stats */
	XFS_GETNEXTQUOTA, /* get disk limits and usage */
};

/*
 * Utility routines
 */
extern char *type_to_string(uint __type);
extern char *form_to_string(uint __form);
extern char *time_to_string(time64_t __time, uint __flags);
extern char *bbs_to_string(uint64_t __v, char *__c, uint __size);
extern char *num_to_string(uint64_t __v, char *__c, uint __size);
extern char *pct_to_string(uint64_t __v, uint64_t __t, char *__c, uint __s);

extern FILE *fopen_write_secure(char *__filename);

/*
 * Various utility routine flags
 */
enum {
	NO_HEADER_FLAG =	0x0001,	/* don't print header */
	VERBOSE_FLAG =		0x0002,	/* increase verbosity */
	HUMAN_FLAG =		0x0004,	/* human-readable values */
	QUOTA_FLAG =		0x0008,	/* uid/gid/prid over-quota (soft) */
	LIMIT_FLAG =		0x0010,	/* uid/gid/prid over-limit (hard) */
	ALL_MOUNTS_FLAG =	0x0020,	/* iterate over every mounted xfs */
	TERSE_FLAG =		0x0040,	/* decrease verbosity */
	HISTOGRAM_FLAG =	0x0080,	/* histogram format output */
	DEFAULTS_FLAG =		0x0100,	/* use value as a default */
	ABSOLUTE_FLAG =		0x0200, /* absolute time, not related to now */
	NO_LOOKUP_FLAG =	0x0400, /* skip name lookups, just report ID */
	GETNEXTQUOTA_FLAG =	0x0800, /* use getnextquota quotactl */
};

/*
 * Identifier (uid/gid/prid) cache routines
 */
#define NMAX 32
extern char *uid_to_name(uint32_t __uid);
extern char *gid_to_name(uint32_t __gid);
extern char *prid_to_name(uint32_t __prid);
extern bool isdigits_only(const char *);

time64_t decode_timer(const struct fs_disk_quota *d, __s32 timer_lo,
		__s8 timer_hi);

#endif /* XFS_QUOTA_QUOTA_H_ */
