/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef XFS_SCRUB_REPAIR_H_
#define XFS_SCRUB_REPAIR_H_

struct action_list {
	struct list_head	list;
	unsigned long long	nr;
	bool			sorted;
};

int action_lists_alloc(size_t nr, struct action_list **listsp);
void action_lists_free(struct action_list **listsp);

void action_list_init(struct action_list *alist);

static inline bool action_list_empty(const struct action_list *alist)
{
	return list_empty(&alist->list);
}

unsigned long long action_list_length(struct action_list *alist);
void action_list_add(struct action_list *dest, struct action_item *item);
void action_list_splice(struct action_list *dest, struct action_list *src);

void action_list_find_mustfix(struct action_list *actions,
		struct action_list *immediate_alist,
		unsigned long long *broken_primaries,
		unsigned long long *broken_secondaries);

/* Passed through to xfs_repair_metadata() */
#define ALP_REPAIR_ONLY		(XRM_REPAIR_ONLY)
#define ALP_COMPLAIN_IF_UNFIXED	(XRM_COMPLAIN_IF_UNFIXED)
#define ALP_NOPROGRESS		(1U << 31)

int action_list_process(struct scrub_ctx *ctx, int fd,
		struct action_list *alist, unsigned int repair_flags);
void action_list_defer(struct scrub_ctx *ctx, xfs_agnumber_t agno,
		struct action_list *alist);
int action_list_process_or_defer(struct scrub_ctx *ctx, xfs_agnumber_t agno,
		struct action_list *alist);

#endif /* XFS_SCRUB_REPAIR_H_ */
