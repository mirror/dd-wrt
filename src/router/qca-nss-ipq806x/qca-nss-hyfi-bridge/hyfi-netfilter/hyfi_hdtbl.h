/*
 * Copyright (c) 2012, 2016, The Linux Foundation. All rights reserved.
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

#ifndef HYFI_HDTBL_H_
#define	HYFI_HDTBL_H_

#include "hyfi_netfilter.h"
#include "hyfi_bridge.h"

/* hybrid default table entry */
struct net_hdtbl_entry {
	struct hlist_node hlist;
	struct net_bridge_port *dst_udp; /* dest port for udp */
	struct net_bridge_port *dst_other; /* dest port for other */

	struct rcu_head rcu;
	mac_addr addr;
	mac_addr id;
	u_int32_t flags;
#define HYFI_HDTBL_STATIC_ENTRY		0x00000001
	struct hyfi_net_bridge * hyfi_br;
};

extern u_int32_t hdtbl_salt __read_mostly;

static inline void hyfi_hd_set_flag(struct net_hdtbl_entry *hd, u_int32_t flag)
{
	hd->flags |= flag;
}

static inline void hyfi_hd_clear_flag(struct net_hdtbl_entry *hd,
		u_int32_t flag)
{
	hd->flags &= ~flag;
}

static inline u_int32_t hyfi_hd_has_flag(const struct net_hdtbl_entry *hd,
		u_int32_t flag)
{
	return ((hd->flags & flag) != 0);
}

static inline int hdtbl_mac_hash(const u_int8_t *mac)
{
	/* use 1 byte of OUI and 3 bytes of NIC */
	u_int32_t key = get_unaligned((u_int32_t *) (mac + 2));
	return jhash_1word(key, hdtbl_salt) & (HD_HASH_SIZE - 1);
}

/* No locking or refcounting, assumes caller has no preempt (rcu_read_lock) */
static inline struct net_hdtbl_entry* __hyfi_hdtbl_get(
		struct hyfi_net_bridge *br, const unsigned char *addr)
{
	struct hlist_node *h;
	struct net_hdtbl_entry *hd;

	os_hlist_for_each_entry_rcu(hd, h, &br->hash_hd[hdtbl_mac_hash(addr)], hlist) {
		if (!compare_ether_addr(hd->addr.addr, addr)) {
			return hd;
		}
	}

	return NULL ;
}

extern int hyfi_hdtbl_init(void);
extern void hyfi_hdtbl_free(void);
extern void hyfi_hdtbl_fini(struct hyfi_net_bridge *br);
extern void hyfi_hdtbl_flush(struct hyfi_net_bridge *br);
extern void hyfi_hdtbl_cleanup(unsigned long arg);
extern int hyfi_hdtbl_fillbuf(struct hyfi_net_bridge *br, void *buf,
		u_int32_t buf_len, u_int32_t offsite, u_int32_t *bytes_written);
extern int hyfi_hdtbl_insert(struct hyfi_net_bridge *br,
		struct net_device *brdev, struct __hdtbl_entry *hde);
extern int hyfi_hdtbl_update(struct hyfi_net_bridge *br,
		struct net_device *brdev, struct __hdtbl_entry *hde);
extern int hyfi_hdtbl_delete(struct hyfi_net_bridge *br,
		const unsigned char *addr);
extern int hyfi_hdtbl_delete_byid(struct hyfi_net_bridge *br,
		const unsigned char *id);
extern struct net_hdtbl_entry *hyfi_hdtbl_get(struct hyfi_net_bridge *br,
		const u_int8_t *addr);
extern struct net_hdtbl_entry *hyfi_hdtbl_find(struct hyfi_net_bridge *br,
		const u_int8_t *addr);
extern void hyfi_hdtbl_delete_by_port(struct hyfi_net_bridge *br,
		const struct net_bridge_port *p);

#endif
