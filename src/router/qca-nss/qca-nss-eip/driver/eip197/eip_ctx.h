/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#ifndef __EIP_CTX_H
#define __EIP_CTX_H

/*
 * eip_ctx_tr
 *	Tranform record list.
 *	TODO: Move this under eip_tun.
 */
struct eip_ctx_tr {
	struct list_head head;
	rwlock_t lock;
	uint16_t count;
	struct {
		uint64_t alloc;
		uint64_t dealloc;
		uint64_t free;
	} stats;
};

/*
 * eip_ctx
 *	Per service HW context
 */
struct eip_ctx {
	struct eip_dma *dma;			/* DMA object to use */
	struct kmem_cache *tk_cache;		/* Token cache for crypto operation & auth key */
	struct kmem_cache *sw_cache;		/* Software desc cache */

	/*
	 * TODO: Move eip_ctx_tr under tun.
	 */
	struct eip_ctx_tr tr;			/* TR list under this context */

	struct kref ref;			/* Reference incremented for each tr */
	enum eip_svc svc;			/* Service for which context is allocated */
	const struct eip_svc_entry *svc_db;	/* Service algorithm database */
	size_t db_size;			/* Size of database */

	struct eip_pdev *ep;			/* EIP platform data */
	struct dentry *dentry;			/* debugfs file node */
};

/*
 * eip_flow_ctx
 *	EIP flow context;
 */
struct eip_tun {
	struct list_head flows;		/* List of flows */
	struct eip_ctx *ctx;	/* EIP context */
};

void eip_ctx_final(struct kref *kref);
void eip_ctx_add_tr(struct eip_ctx *ctx, struct eip_tr *tr);
void eip_ctx_del_tr(struct eip_ctx *ctx, struct eip_tr *tr);
const struct eip_svc_entry *eip_ctx_algo_lookup(struct eip_ctx *ctx, char *algo_name);

/*
 * eip_ctx_ref()
 *	Increment context reference.
 */
static inline struct eip_ctx *eip_ctx_ref(struct eip_ctx *ctx)
{
	kref_get(&ctx->ref);
	return ctx;
}

/*
 * eip_ctx_deref()
 *	Decrement context reference.
 */
static inline void eip_ctx_deref(struct eip_ctx *ctx)
{
	kref_put(&ctx->ref, eip_ctx_final);
}

#endif /* __EIP_CTX_H */
