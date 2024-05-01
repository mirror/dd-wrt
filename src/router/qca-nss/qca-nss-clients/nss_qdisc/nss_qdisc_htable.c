/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 **************************************************************************
 */

#include "nss_qdisc.h"

/*
 * Note: The table size 1024 ensures atleast page size memory is allocated for 32 bit binary.
 */
#define NSS_QDISC_LIST_TABLE_INIT_SIZE 1024

/*
 * nss_qdisc_htable_entry_compute_hash()
 * 	Calculate hash for given qos_tag
 */
static inline uint32_t nss_qdisc_htable_entry_compute_hash(uint32_t qos_tag, uint32_t mask)
{
	/*
	 * Since this list is for qdiscs and not classes the lower 16bits
	 * are ignored for computing hash.
	 */
	return ((qos_tag >> 16) ^ (qos_tag >> 24)) & mask;
}

/*
 * nss_qdisc_htable_entry_lookup()
 * 	Get nss_qdisc corresponding to qos_tag from hash table
 */
struct nss_qdisc *nss_qdisc_htable_entry_lookup(struct nss_qdisc_htable *nqt, uint32_t qos_tag)
{
	struct nss_qdisc *nq;
	uint32_t slot = nss_qdisc_htable_entry_compute_hash(qos_tag, nqt->htsize - 1);

	spin_lock_bh(&nqt->lock);
	hlist_for_each_entry(nq, &nqt->htable[slot], hlist) {
		if (nq->qos_tag != qos_tag) {
			continue;
		}
		spin_unlock_bh(&nqt->lock);
		return nq;
	}
	spin_unlock_bh(&nqt->lock);
	return NULL;
}

/*
 * nss_qdisc_htable_entry_del()
 * 	Del nss_qdisc from hash table
 */
void nss_qdisc_htable_entry_del(struct nss_qdisc_htable *nqt, struct nss_qdisc *nq)
{
	spin_lock_bh(&nqt->lock);
	nqt->count--;
	hlist_del(&nq->hlist);
	spin_unlock_bh(&nqt->lock);
}

/*
 * nss_qdisc_htable_entry_add()
 * 	Add nss_qdisc to hash table
 */
void nss_qdisc_htable_entry_add(struct nss_qdisc_htable *nqt, struct nss_qdisc *nq)
{
	uint32_t hslot = nss_qdisc_htable_entry_compute_hash(nq->qos_tag, nqt->htsize - 1);
	INIT_HLIST_NODE(&nq->hlist);

	spin_lock_bh(&nqt->lock);
	nqt->count++;
	hlist_add_head(&nq->hlist, &nqt->htable[hslot]);
	spin_unlock_bh(&nqt->lock);
}

/*
 * nss_qdisc_htable_hlist_init()
 * 	Initialize list head of each slot of allocated table
 */
static inline void nss_qdisc_htable_hlist_init(struct hlist_head *hlh, uint32_t size)
{
	/*
	 * Initialize list head of each slot
	 */
	while (size) {
		INIT_HLIST_HEAD(&hlh[--size]);
	}
}

/*
 * nss_qdisc_htable_alloc()
 * 	Allocate hash table
 */
static struct hlist_head *nss_qdisc_htable_alloc(struct nss_qdisc_htable *nqt, uint32_t size)
{
	struct hlist_head *hlh;
	uint32_t msize = size * sizeof(struct hlist_head);

	hlh = (struct hlist_head *)__get_free_pages(GFP_KERNEL, get_order(msize));
	if (!hlh) {
		nss_qdisc_warning("%p: Qdisc list hash table memory allocation failed, size: %u\n", nqt, msize);
		return NULL;
	}

	nqt->htsize = size;
	nss_qdisc_info("%p: Qdisc list hash table memory allocation of size %u sucessful, for %u members\n", nqt, msize, size);

	return hlh;
}

/*
 * nss_qdisc_htable_free()
 * 	Free hash table. The caller should take the lock on the table
 */
static void nss_qdisc_htable_free(struct nss_qdisc_htable *nqt, struct hlist_head *hlh, uint32_t size)
{
	uint32_t msize = size * sizeof(struct hlist_head);

	if (!hlh) {
		nss_qdisc_warning("%p: Freeing invalid memory \n", nqt);
		return;
	}

	free_pages((unsigned long)hlh, get_order(msize));
}

/*
 * nss_qdisc_htable_resize_needed()
 *	Check if table needs resize
 */
static uint32_t nss_qdisc_htable_resize_needed(struct nss_qdisc_htable *nqt)
{
	/*
	 * Check if we have our table almost full,
	 * allocate a new htable if so.
	 */
	if (nqt->htsize * 4 > nqt->count * 5) {
		return 0;
	}

	/*
	 * Double the hash table size
	 */
	return nqt->htsize * 2;
}

/*
 * nss_qdisc_htable_resize()
 *	Check and grow the hash table size if needed
 */
void nss_qdisc_htable_resize(struct Qdisc *sch, struct nss_qdisc_htable *nqt)
{
	struct nss_qdisc *nq;
	uint32_t prev_htsize, new_htsize, new_mask, new_slot;
	uint32_t slot = prev_htsize = nqt->htsize;
	struct hlist_head *hlh, *nhlh;
	struct hlist_node *tmp;

	new_htsize = nss_qdisc_htable_resize_needed(nqt);
	if (!new_htsize) {
		return;
	}

	hlh = nqt->htable;
	nhlh = nss_qdisc_htable_alloc(nqt, new_htsize);
	if (!nhlh) {
		nss_qdisc_info("%p: nss_qdisc failed to allocate new htable for size %u\n", nqt, new_htsize);
		return;
	}

	nss_qdisc_htable_hlist_init(nhlh, new_htsize);
	new_mask = new_htsize - 1;
	spin_lock_bh(&nqt->lock);
	while (slot) {
		/*
		 * Populate new table
		 */
		hlist_for_each_entry_safe(nq, tmp, &hlh[--slot], hlist) {
			new_slot = nss_qdisc_htable_entry_compute_hash(nq->qos_tag, new_mask);
			hlist_add_head(&nq->hlist, &nhlh[new_slot]);
		}
	}

	/*
	 * Free the old table
	 */
	nss_qdisc_htable_free(nqt, hlh, prev_htsize);

	/*
	 * Attach the new table
	 */
	nqt->htable = nhlh;
	spin_unlock_bh(&nqt->lock);
}

/*
 * nss_qdisc_htable_init()
 *	Allocate and initialise the hash table
 */
bool nss_qdisc_htable_init(struct nss_qdisc_htable *nqt)
{
	/*
	 * Initialize table for eight qdisc nodes, it will grown if
	 * needed as the number of nodes increase
	 */
	nqt->htable = nss_qdisc_htable_alloc(nqt, NSS_QDISC_LIST_TABLE_INIT_SIZE);
	if (!nqt->htable) {
		nss_qdisc_warning(" %p: nss_qdisc table init failed \n", nqt);
		return false;
	}

	nqt->count = 0;
	nss_qdisc_htable_hlist_init(nqt->htable, NSS_QDISC_LIST_TABLE_INIT_SIZE);
	return true;
}

/*
 * nss_qdisc_htable_dealloc()
 *	Destroy the hash table. The entries in the hash table are
 *	expected to be freed prior to this
 */
void nss_qdisc_htable_dealloc(struct nss_qdisc_htable *nqt)
{
	spin_lock_bh(&nqt->lock);
	nss_qdisc_htable_free(nqt, nqt->htable, nqt->htsize);
	spin_unlock_bh(&nqt->lock);
}

