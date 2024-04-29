/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE
 **************************************************************************
 */

/*
 * nss_tlsmgr_tun.h
 */

#ifndef __NSS_TLSMGR_TUN_H_
#define __NSS_TLSMGR_TUN_H_

#define NSS_TLSMGR_MAX_HDR_SZ \
	(sizeof(struct ipv6hdr) + sizeof(struct tcphdr) + sizeof(struct tlshdr) + AES_BLOCK_SIZE)
#define NSS_TLSMGR_MAX_TRAILER_SZ (AES_BLOCK_SIZE + SHA256_DIGEST_SIZE + 1)
#define NSS_TLSMGR_MAX_HEADROOM NSS_TLSMGR_MAX_HDR_SZ
#define NSS_TLSMGR_MAX_TAILROOM NSS_TLSMGR_MAX_TRAILER_SZ

#define NSS_TLSMGR_TUN_DECONGEST_TICKS 100

/*
 * Tunnel Event notification
 */
struct nss_tlsmgr_notify {
	nss_tlsmgr_notify_callback_t cb; 	/* Callback function */
	struct timer_list timer; 		/* Timer for invoking the callback */
	unsigned long ticks; 			/* User programmed time interval */
	void *app_data; 			/* Callback context */
};

/*
 * Decongestion Event notification
 */
struct nss_tlsmgr_decongest {
	nss_tlsmgr_decongest_callback_t cb;	/* Callback function */
	struct timer_list timer; 		/* Timer for invoking the callback */
	unsigned long ticks; 			/* User programmed time interval */
	void *app_data; 			/* Callback context */
};

/*
 * TLS tunnel object
 */
struct nss_tlsmgr_tun  {
	struct net_device *dev;   			/* Tunnel netdevice. */
	rwlock_t lock;					/* Tunnel lock. */
	atomic_t pkt_pending;				/* Packets not retured to host. */
	struct dentry *dentry;    			/* Debugfs directory for tunnel stats. */
	struct list_head free_list;			/* List of inactive TLS record(s). */
	struct work_struct free_work;			/* Work scheduled for deletion. */
	struct nss_tlsmgr_notify notify; 		/* Period notification to user */
	struct nss_tlsmgr_decongest decongest;		/* Decongestion notification to user */
	struct nss_tlsmgr_ctx ctx_enc;     		/* Encapsulation context. */
	struct nss_tlsmgr_ctx ctx_dec;			/* Decapsulation context. */
};

#endif /* !__NSS_TLSMGR_TUN_H_ */
