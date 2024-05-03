/*
 * Copyright (c) 2014 - 2015, The Linux Foundation. All rights reserved.
 *
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
 */

/*
 * rfs_cm.h
 *	RFS connection manager - connection manager
 */

#ifndef __RFS_CM_H
#define __RFS_CM_H
struct rfs_cm_ipv4_connection {
        struct hlist_node conn_hlist;
        struct hlist_node snat_hlist;
        struct hlist_node dnat_hlist;
	struct rcu_head rcu;
        uint8_t protocol;
        __be32 orig_src_ip;
        __be32 orig_dest_ip;
        __be16 orig_src_port;
        __be16 orig_dest_port;
        __be32 reply_src_ip;
        __be32 reply_dest_ip;
        __be16 reply_src_port;
        __be16 reply_dest_port;
	uint32_t orig_rxhash;
	uint32_t reply_rxhash;
	uint32_t flag;
        uint16_t cpu;
};

#define RFS_CM_FLAG_SNAT 0x0001
#define RFS_CM_FLAG_DNAT 0x0002
void rfs_cm_connection_destroy_all(void);
int rfs_cm_update_rules(__be32 ipaddr, uint16_t cpu);

int rfs_cm_start(void);
int rfs_cm_stop(void);
int rfs_cm_init(void);
void rfs_cm_exit(void);
#endif
