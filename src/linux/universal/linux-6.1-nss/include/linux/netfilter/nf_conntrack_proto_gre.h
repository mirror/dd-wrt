/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _CONNTRACK_PROTO_GRE_H
#define _CONNTRACK_PROTO_GRE_H
#include <asm/byteorder.h>
#include <net/gre.h>
#include <net/pptp.h>

struct nf_ct_gre {
	unsigned int stream_timeout;
	unsigned int timeout;
};

#include <net/netfilter/nf_conntrack_tuple.h>

struct nf_conn;

/* structure for original <-> reply keymap */
struct nf_ct_gre_keymap {
	struct list_head list;
	struct nf_conntrack_tuple tuple;
	struct rcu_head rcu;
};

/* add new tuple->key_reply pair to keymap */
int nf_ct_gre_keymap_add(struct nf_conn *ct, enum ip_conntrack_dir dir,
			 struct nf_conntrack_tuple *t);

void nf_ct_gre_keymap_flush(struct net *net);
/* delete keymap entries */
void nf_ct_gre_keymap_destroy(struct nf_conn *ct);

bool gre_pkt_to_tuple(const struct sk_buff *skb, unsigned int dataoff,
		      struct net *net, struct nf_conntrack_tuple *tuple);

/* QCA NSS ECM Support - Start */
/* GRE is a mess: Four different standards */
struct gre_hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
        __u16   rec:3,
                srr:1,
                seq:1,
                key:1,
                routing:1,
                csum:1,
                version:3,
                reserved:4,
                ack:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
        __u16   csum:1,
                routing:1,
                key:1,
                seq:1,
                srr:1,
                rec:3,
                ack:1,
                reserved:4,
                version:3;
#else
#error "Adjust your <asm/byteorder.h> defines"
#endif
        __be16  protocol;
};
/* QCA NSS ECM Support - End */


#endif /* _CONNTRACK_PROTO_GRE_H */
