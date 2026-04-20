/*
 **************************************************************************
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/* nss_ipsecmgr_ref.c
 *	IPSec Manager reference routines
 */
#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/of.h>
#include <linux/ipv6.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/etherdevice.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <linux/atomic.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/ip6_route.h>
#include <net/esp.h>
#include <net/xfrm.h>
#include <net/icmp.h>

#include <crypto/aead.h>
#include <crypto/skcipher.h>
#include <crypto/internal/hash.h>

#include <nss_api_if.h>
#include <nss_ipsec_cmn.h>
#include <nss_ipsecmgr.h>
#include <nss_cryptoapi.h>

#ifdef NSS_IPSECMGR_PPE_SUPPORT
#include <ref/ref_vsi.h>
#endif

#include "nss_ipsecmgr_ref.h"
#include "nss_ipsecmgr_flow.h"
#include "nss_ipsecmgr_sa.h"
#include "nss_ipsecmgr_ctx.h"
#include "nss_ipsecmgr_tunnel.h"
#include "nss_ipsecmgr_priv.h"

extern struct nss_ipsecmgr_drv *ipsecmgr_drv;

/*
 * nss_ipsecmgr_ref_no_del()
 *	dummy functions for object owner when there is no delete
 */
static void nss_ipsecmgr_ref_no_del(struct nss_ipsecmgr_ref *ref)
{
	nss_ipsecmgr_trace("%px: ref_no_del triggered\n", ref);
	return;
}

/*
 * nss_ipsecmgr_ref_no_free()
 *	dummy functions for object owner when there is no free
 */
static void nss_ipsecmgr_ref_no_free(struct nss_ipsecmgr_ref *ref)
{
	nss_ipsecmgr_trace("%px: ref_no_free triggered\n", ref);
	return;
}

/*
 * nss_ipsecmgr_ref_no_print_len()
 *	dummy functions for object owner when there is no stats length
 */
static ssize_t nss_ipsecmgr_ref_no_print_len(struct nss_ipsecmgr_ref *ref)
{
	nss_ipsecmgr_trace("%px: ref_no_free triggered\n", ref);
	return 0;
}

/*
 * nss_ipsecmgr_ref_no_print()
 *	dummy functions for object owner when there is no stats dump
 */
static ssize_t nss_ipsecmgr_ref_no_print(struct nss_ipsecmgr_ref *ref, char *buf)
{
	nss_ipsecmgr_trace("%px: ref_no_free triggered\n", ref);
	return 0;
}

/*
 * nss_ipsecmgr_ref_init()
 *	initiaize the reference object
 */
void nss_ipsecmgr_ref_init(struct nss_ipsecmgr_ref *ref, nss_ipsecmgr_ref_method_t del, nss_ipsecmgr_ref_method_t free)
{
	INIT_LIST_HEAD(&ref->head);
	INIT_LIST_HEAD(&ref->node);

	ref->id = 0;
	ref->parent = NULL;
	ref->free = free ? free : nss_ipsecmgr_ref_no_free;
	ref->del = del ? del : nss_ipsecmgr_ref_no_del;
	ref->print_len = nss_ipsecmgr_ref_no_print_len;
	ref->print = nss_ipsecmgr_ref_no_print;
}

/*
 * nss_ipsecmgr_ref_init_print
 *	Initialize print methods
 */
void nss_ipsecmgr_ref_init_print(struct nss_ipsecmgr_ref *ref, nss_ipsecmgr_ref_get_method_t print_len,
				nss_ipsecmgr_ref_print_method_t print)
{
	ref->print_len = print_len;
	ref->print = print;
}

/*
 * nss_ipsecmgr_ref_print()
 *	Print reference node information and its children
 */
ssize_t nss_ipsecmgr_ref_print(struct nss_ipsecmgr_ref *ref, char *buf)
{
	struct nss_ipsecmgr_ref *entry;
	size_t len = 0;

	/*
	 * DEBUG check to see if the lock is taken before touching the list
	 */
	nss_ipsecmgr_write_lock_is_held(&ipsecmgr_drv->lock);

	len += ref->print(ref, buf);

	list_for_each_entry(entry, &ref->head, node) {
		len += nss_ipsecmgr_ref_print(entry, buf + len);
	}

	return len;
}

/*
 * nss_ipsecmgr_ref_print_len()
 *	Get print length for the reference node and its children
 */
ssize_t nss_ipsecmgr_ref_print_len(struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_ref *entry;
	size_t total_len = 0;

	/*
	 * DEBUG check to see if the lock is taken before touching the list
	 */
	nss_ipsecmgr_write_lock_is_held(&ipsecmgr_drv->lock);

	list_for_each_entry(entry, &ref->head, node) {
		total_len += nss_ipsecmgr_ref_print_len(entry);
	}

	return total_len + ref->print_len(ref);
}

/*
 * nss_ipsecmgr_ref_del()
 *	Delete child reference and add it to zombie list
 *
 * Note: If, the "ref" has child references then it
 * will walk the child reference chain first and issue
 * free for each of the associated "child ref" objects.
 * At the end it will invoke free for the "parent" ref
 * object.
 *
 * +-------+
 * |  tun0 |
 * +-------+
 *     |
 *     V
 * +-------+    +-------+    +-------+
 * |  SA1  |--->|  SA2  |--->|  SA3  |
 * +-------+    +-------+    +-------+
 *     |
 *     V
 * +-------+    +-------+    +-------+
 * | Flow1 |--->| Flow2 |--->| Flow4 |
 * +-------+    +-------+    +-------+
 */
void nss_ipsecmgr_ref_del(struct nss_ipsecmgr_ref *ref, struct list_head *free_q)
{
	struct nss_ipsecmgr_ref *entry;

	/*
	 * DEBUG check to see if the lock is taken before touching the list
	 */
	nss_ipsecmgr_write_lock_is_held(&ipsecmgr_drv->lock);

	while (!list_empty(&ref->head)) {
		entry = list_first_entry(&ref->head, struct nss_ipsecmgr_ref, node);
		nss_ipsecmgr_ref_del(entry, free_q);
	}

	list_del_init(&ref->node);
	ref->del(ref);

	/*
	 * If, free list is valid then add the reference node to zombie
	 * list for delayed free
	 */
	if (free_q) {
		list_add_tail(&ref->node, free_q);
	}
}

/*
 * nss_ipsecmgr_ref_add()
 *	add child reference to parent chain
 */
void nss_ipsecmgr_ref_add(struct nss_ipsecmgr_ref *child, struct nss_ipsecmgr_ref *parent)
{
	/*
	 * DEBUG check to see if the lock is taken before touching the list
	 */
	nss_ipsecmgr_write_lock_is_held(&ipsecmgr_drv->lock);

	/*
	 * if child is already part of an existing chain then remove it before
	 * adding it to the new one. In case this is a new entry then the list
	 * init during alloc would ensure that the "del_init" operation results
	 * in a no-op
	 */
	list_del_init(&child->node);
	list_add(&child->node, &parent->head);

	child->parent = parent;
}
