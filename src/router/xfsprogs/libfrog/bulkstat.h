/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 Oracle, Inc.
 * All Rights Reserved.
 */
#ifndef __LIBFROG_BULKSTAT_H__
#define __LIBFROG_BULKSTAT_H__

/* Bulkstat wrappers */
struct xfs_bstat;
int xfrog_bulkstat_single(struct xfs_fd *xfd, uint64_t ino, unsigned int flags,
		struct xfs_bulkstat *bulkstat);
int xfrog_bulkstat(struct xfs_fd *xfd, struct xfs_bulkstat_req *req);

int xfrog_bulkstat_alloc_req(uint32_t nr, uint64_t startino,
		struct xfs_bulkstat_req **preq);
int xfrog_bulkstat_v5_to_v1(struct xfs_fd *xfd, struct xfs_bstat *bs1,
		const struct xfs_bulkstat *bstat);
void xfrog_bulkstat_v1_to_v5(struct xfs_fd *xfd, struct xfs_bulkstat *bstat,
		const struct xfs_bstat *bs1);

void xfrog_bulkstat_set_ag(struct xfs_bulkstat_req *req, uint32_t agno);

struct xfs_inogrp;
int xfrog_inumbers(struct xfs_fd *xfd, struct xfs_inumbers_req *req);

int xfrog_inumbers_alloc_req(uint32_t nr, uint64_t startino,
		struct xfs_inumbers_req **preq);
void xfrog_inumbers_set_ag(struct xfs_inumbers_req *req, uint32_t agno);
void xfrog_inumbers_v5_to_v1(struct xfs_inogrp *ig1,
		const struct xfs_inumbers *ig);
void xfrog_inumbers_v1_to_v5(struct xfs_inumbers *ig,
		const struct xfs_inogrp *ig1);

#endif	/* __LIBFROG_BULKSTAT_H__ */
