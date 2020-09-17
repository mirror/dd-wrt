// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef XFS_SCRUB_COUNTER_H_
#define XFS_SCRUB_COUNTER_H_

struct ptcounter;
int ptcounter_alloc(size_t nr, struct ptcounter **pp);
void ptcounter_free(struct ptcounter *ptc);
int ptcounter_add(struct ptcounter *ptc, int64_t nr);
int ptcounter_value(struct ptcounter *ptc, uint64_t *sum);

#endif /* XFS_SCRUB_COUNTER_H_ */
