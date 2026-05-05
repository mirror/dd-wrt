/*
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __EIP_CRYPTO_H
#define __EIP_CRYPTO_H

#include <linux/completion.h>
#include <linux/debugfs.h>
#include <linux/vmalloc.h>

#include <eip.h>

#define EIP_CRYPTO_STATS_STR_LEN 20U
#define EIP_CRYPTO_STATS_MAX 16U

/*
 * eip_crypto_tfm_stats
 *	Common stats for all ciphers.
 */
struct eip_crypto_tfm_stats {
	uint64_t tx_reqs;		/* Total tx requests. */
	uint64_t tx_bytes;		/* Total tx bytes. */
	uint64_t tx_failures;		/* Total tx failures. */
	uint64_t rx_reqs;		/* Total rx requests. */
	uint64_t rx_bytes;		/* Total rx bytes. */
	uint64_t rx_failures;		/* Total rx failures. */
	uint64_t hw_err;		/* Hardware errors. */
	uint64_t invalid_key;		/* Invalid key error. */
	uint64_t mem_alloc_err;		/* Memory allocation error. */
	uint64_t tr_alloc_err;		/* TR allocation error. */
	uint64_t inactive_tfm;		/* TFM is inactive. */
	uint64_t invalid_len;		/* Invalid supported len. */
};

/*
 * eip_crypto_tfm_ctx
 *	Context per transformation.
 */
struct eip_crypto_tfm_ctx {
	struct list_head entry;		/* List of contexts. */
	void __rcu *tr;			/* Reference to TR. */
	atomic_t usage_count;		/* TFM usage count. */
	atomic_t active;		/* TFM state(active/inactive). */

	struct completion complete;	/* Completion object waiting for outstanding events.*/
	struct eip_crypto_tfm_stats __percpu *stats;	/* Statistics per transform record. */
};

/*
 * eip_crypto_tfm_head
 *	TFM list
 */
struct eip_crypto_tfm_head {
	struct list_head head;		/* List of TFM. */
	rwlock_t lock;			/* Lock for list operations. */
	atomic_t tfm_count;		/* Number of TFMs in use. */
};

/*
 * eip_crypto
 *	Global instance per crypto service (AEAD, SKCIPHER, AHASH).
 */
struct eip_crypto {
	struct eip_crypto_tfm_head tfms;	/* TFM list. */
	struct eip_crypto_tfm_stats __percpu *stats;
					/* Combined stats for all tfms .*/
	struct eip_ctx *dma_ctx;	/* DMA context of cipher. */

	struct completion complete;	/* Completion object for entire cipher. */
	struct dentry *root;		/* Debugfs entry per cipher. */
};

extern int eip_crypto_aead_init(struct device_node *node);
extern void eip_crypto_aead_exit(struct device_node *node);
extern int eip_crypto_skcipher_init(struct device_node *node);
extern void eip_crypto_skcipher_exit(struct device_node *node);
extern int eip_crypto_ahash_init(struct device_node *node);
extern void eip_crypto_ahash_exit(struct device_node *node);

/*
 * eip_crypto_tfm_add_tfmctx()
 *	Adds tfm to global tfm list.
 */
static inline void eip_crypto_add_tfmctx(struct eip_crypto *ctx, struct eip_crypto_tfm_ctx *tfm_ctx)
{
	struct list_head *entry = &tfm_ctx->entry;

	write_lock_bh(&ctx->tfms.lock);
	list_add(entry, &ctx->tfms.head);
	write_unlock_bh(&ctx->tfms.lock);

	/*
	 * Increment TFM count.
	 */
	atomic_inc(&ctx->tfms.tfm_count);
}

/*
 * eip_crypto_tfm_del_from_list()
 *	Removes tfm from global tfm list.
 */
static inline void eip_crypto_del_tfmctx(struct eip_crypto *ctx, struct eip_crypto_tfm_ctx *tfm_ctx)
{
	struct list_head *entry = &tfm_ctx->entry;

	write_lock_bh(&ctx->tfms.lock);
	list_del(entry);
	write_unlock_bh(&ctx->tfms.lock);

	/*
	 * Increment TFM count.
	 */
	atomic_dec(&ctx->tfms.tfm_count);
}

/*
 * eip_crypto_get_summary_stats()
 *	Generates summary of stats per CPU.
 */
void eip_crypto_get_summary_stats(struct eip_crypto_tfm_stats *tfm_stats, struct eip_crypto_tfm_stats *stats);

#endif /* __EIP_CRYPTO_H */
