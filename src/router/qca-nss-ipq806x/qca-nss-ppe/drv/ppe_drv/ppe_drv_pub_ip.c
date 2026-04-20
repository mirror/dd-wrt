/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/kref.h>
#include <fal/fal_ip.h>
#include <fal/fal_init.h>
#include "ppe_drv.h"

#if (PPE_DRV_DEBUG_LEVEL == 3)
/*
 * ppe_drv_pub_ip_dump()
 *	Dumps public ip related tables.
 */
static void ppe_drv_pub_ip_dump(struct ppe_drv_pub_ip *pub_ip)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_ip_pub_addr_t pub_ip_cfg;
	sw_error_t err;

	err = fal_ip_pub_addr_get(PPE_DRV_SWITCH_ID, pub_ip->index, &pub_ip_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: public ip query failed for pub_ip index: %d", p, pub_ip->index);
		return;
	}

	ppe_drv_trace("%p: public ip address: %pl4", p, &pub_ip_cfg.pub_ip_addr);
}
#else
static void ppe_drv_pub_ip_dump(struct ppe_drv_pub_ip *pub_ip)
{
}
#endif

/*
 * ppe_drv_pub_ip_free()
 *	Free given IP from PUB_IP_ADDR_TBL.
 */
static void ppe_drv_pub_ip_free(struct kref *kref)
{
	struct ppe_drv_pub_ip *pub_ip = container_of(kref, struct ppe_drv_pub_ip, ref);
	fal_ip_pub_addr_t pub_ip_cfg = { 0 };

	/*
	 * Clear PUB_IP_ADDR table entry.
	 */
	if (fal_ip_pub_addr_set(PPE_DRV_SWITCH_ID, pub_ip->index, &pub_ip_cfg) != SW_OK) {
		ppe_drv_warn("%p: unable to clear pub_ip table\n", pub_ip);
	}

	pub_ip->ip_addr = 0;
}

/*
 * ppe_drv_pub_ip_deref()
 *	Deref public ip
 */
bool ppe_drv_pub_ip_deref(struct ppe_drv_pub_ip *pub_ip)
{
	if (kref_put(&pub_ip->ref, ppe_drv_pub_ip_free)) {
		ppe_drv_trace("%p: pub_ip reference goes down to 0\n", pub_ip);
		return true;
	}

	ppe_drv_trace("%p: pub_ip reference: %d\n", pub_ip, kref_read(&pub_ip->ref));
	return false;
}

/*
 * ppe_drv_pub_ip_ref()
 *	Increments ref count for pub ip address entry.
 */
struct ppe_drv_pub_ip *ppe_drv_pub_ip_ref(struct ppe_drv_pub_ip *pub_ip)
{
	kref_get(&pub_ip->ref);
	return pub_ip;
}

/*
 * ppe_drv_pub_ip_get_and_ref()
 *	Adds an IPv4 addr to PUB_IP_ADDR_TBL if it does not exist and returns index.
 *
 * Note: the pub_ip table is used for NAT purposes and is therefore needed only for IPv4.
 */
struct ppe_drv_pub_ip *ppe_drv_pub_ip_get_and_ref(uint32_t ip_addr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_pub_ip *pub_ip;
	fal_ip_pub_addr_t pub_ip_cfg;
	int32_t free_index = -1;
	uint16_t i;

	ppe_drv_assert(spin_is_locked(&p->lock), "%px: ppe drv lock is not held\n", p);

	/*
	 * Return index, if IP address already exists.
	 */
	for (i = 0; i < p->pub_ip_num; i++) {

		/*
		 * Return index if there is an IP match.
		 */
		struct ppe_drv_pub_ip *pub_ip = &p->pub_ip[i];
		if (kref_read(&pub_ip->ref) && (pub_ip->ip_addr == ip_addr)) {
			ppe_drv_pub_ip_ref(pub_ip);
			ppe_drv_trace("%p: ip_addr %x already exists at index %d ",
					pub_ip, ip_addr, i);
			return pub_ip;
		}

		/*
		 * Save first free index to be used later.
		 */
		if ((free_index < 0) && !kref_read(&pub_ip->ref)) {
			free_index = i;
		}
	}

	/*
	 * If there is no space left in pub-ip table, return failure.
	 */
	if (free_index < 0) {
		ppe_drv_warn("%p: pub ip table full, cannot insert ip %x", p, ip_addr);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_pubip_full);
		return NULL;
	}

	pub_ip = &p->pub_ip[free_index];

	/*
	 * Update PUB_IP_ADDR table entry.
	 */
	pub_ip_cfg.pub_ip_addr = ip_addr;
	if (fal_ip_pub_addr_set(PPE_DRV_SWITCH_ID, pub_ip->index, &pub_ip_cfg) != SW_OK) {
		ppe_drv_warn("%p: pub_ip configuration failed for ip_addr(%x) for idx (%d)", p, ip_addr, free_index);
		return NULL;
	}

	pub_ip->ip_addr = ip_addr;
	kref_init(&pub_ip->ref);
	ppe_drv_pub_ip_dump(pub_ip);

	return pub_ip;
}

/*
 * ppe_drv_pub_ip_entries_free()
 *	Free pub_ip table entries if it was allocated.
 */
void ppe_drv_pub_ip_entries_free(struct ppe_drv_pub_ip *pub_ip)
{
	vfree(pub_ip);
}

/*
 * ppe_drv_pub_ip_entries_alloc()
 *	Allocate and initialize pub_ip entries.
 */
struct ppe_drv_pub_ip *ppe_drv_pub_ip_entries_alloc()
{
	struct ppe_drv_pub_ip *pub_ip;
	struct ppe_drv *p = &ppe_drv_gbl;
	uint16_t i;

	pub_ip = vzalloc(sizeof(struct ppe_drv_pub_ip) * p->pub_ip_num);
	if (!pub_ip) {
		ppe_drv_warn("%p: failed to allocate pub_ip entries", p);
		return NULL;
	}

	for (i = 0; i < p->pub_ip_num; i++) {
		pub_ip[i].index = i;
	}

	return pub_ip;
}
