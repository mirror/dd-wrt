/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef XFS_SCRUB_DESCR_H_
#define XFS_SCRUB_DESCR_H_

typedef int (*descr_fn)(struct scrub_ctx *ctx, char *buf, size_t buflen,
			void *data);

struct descr {
	struct scrub_ctx	*ctx;
	descr_fn		fn;
	void			*where;
};

#define DEFINE_DESCR(_name, _ctx, _fn) \
	struct descr _name = { .ctx = (_ctx), .fn = (_fn) }

const char *__descr_render(struct descr *dsc, const char *file, int line);
#define descr_render(dsc) __descr_render((dsc), __FILE__, __LINE__)

void descr_set(struct descr *dsc, void *where);

int descr_init_phase(struct scrub_ctx *ctx, unsigned int nr_threads);
void descr_end_phase(void);

#endif /* XFS_SCRUB_DESCR_H_ */
