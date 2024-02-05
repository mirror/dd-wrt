/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef __XFS_REPAIR_QUOTACHECK_H__
#define __XFS_REPAIR_QUOTACHECK_H__

void quotacheck_skip(void);
void quotacheck_adjust(struct xfs_mount *mp, xfs_ino_t ino);
void quotacheck_verify(struct xfs_mount *mp, xfs_dqtype_t type);
uint16_t quotacheck_results(void);
int quotacheck_setup(struct xfs_mount *mp);
void quotacheck_teardown(void);

#endif /* __XFS_REPAIR_QUOTACHECK_H__ */
