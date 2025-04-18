/*
 *
 *	Generic internet FLOW.
 *
 */

#ifndef _NET_FLOW_H
#define _NET_FLOW_H

#include <linux/socket.h>
#include <linux/in6.h>
#include <linux/atomic.h>
#include <net/flow_dissector.h>
#include <linux/uidgid.h>

/*
 * ifindex generation is per-net namespace, and loopback is
 * always the 1st device in ns (see net_dev_init), thus any
 * loopback device should get ifindex 1
 */

#define LOOPBACK_IFINDEX	1

struct flowi_tunnel {
	__be64			tun_id;
};

struct flowi_common {
	int	flowic_oif;
	int	flowic_iif;
	__u32	flowic_mark;
	__u8	flowic_tos;
	__u8	flowic_scope;
	__u8	flowic_proto;
	__u8	flowic_flags;
#define FLOWI_FLAG_ANYSRC		0x01
#define FLOWI_FLAG_KNOWN_NH		0x02
#define FLOWI_FLAG_SKIP_NH_OIF		0x04
	__u32	flowic_secid;
	struct flowi_tunnel flowic_tun_key;
	kuid_t  flowic_uid;
};

union flowi_uli {
	struct {
		__be16	dport;
		__be16	sport;
	} ports;

	struct {
		__u8	type;
		__u8	code;
	} icmpt;

	struct {
		__le16	dport;
		__le16	sport;
	} dnports;

	__be32		spi;
	__be32		gre_key;

	struct {
		__u8	type;
	} mht;
};

struct flowi4 {
	struct flowi_common	__fl_common;
#define flowi4_oif		__fl_common.flowic_oif
#define flowi4_iif		__fl_common.flowic_iif
#define flowi4_mark		__fl_common.flowic_mark
#define flowi4_tos		__fl_common.flowic_tos
#define flowi4_scope		__fl_common.flowic_scope
#define flowi4_proto		__fl_common.flowic_proto
#define flowi4_flags		__fl_common.flowic_flags
#define flowi4_secid		__fl_common.flowic_secid
#define flowi4_tun_key		__fl_common.flowic_tun_key
#define flowi4_uid		__fl_common.flowic_uid

	/* (saddr,daddr) must be grouped, same order as in IP header */
	__be32			saddr;
	__be32			daddr;

	union flowi_uli		uli;
#define fl4_sport		uli.ports.sport
#define fl4_dport		uli.ports.dport
#define fl4_icmp_type		uli.icmpt.type
#define fl4_icmp_code		uli.icmpt.code
#define fl4_ipsec_spi		uli.spi
#define fl4_mh_type		uli.mht.type
#define fl4_gre_key		uli.gre_key
} __attribute__((__aligned__(BITS_PER_LONG/8)));

static inline void flowi4_init_output(struct flowi4 *fl4, int oif,
				      __u32 mark, __u8 tos, __u8 scope,
				      __u8 proto, __u8 flags,
				      __be32 daddr, __be32 saddr,
				      __be16 dport, __be16 sport)
{
	fl4->flowi4_oif = oif;
	fl4->flowi4_iif = LOOPBACK_IFINDEX;
	fl4->flowi4_mark = mark;
	fl4->flowi4_tos = tos;
	fl4->flowi4_scope = scope;
	fl4->flowi4_proto = proto;
	fl4->flowi4_flags = flags;
	fl4->flowi4_secid = 0;
	fl4->flowi4_tun_key.tun_id = 0;
	fl4->daddr = daddr;
	fl4->saddr = saddr;
	fl4->fl4_dport = dport;
	fl4->fl4_sport = sport;
}

/* Reset some input parameters after previous lookup */
static inline void flowi4_update_output(struct flowi4 *fl4, int oif, __u8 tos,
					__be32 daddr, __be32 saddr)
{
	fl4->flowi4_oif = oif;
	fl4->flowi4_tos = tos;
	fl4->daddr = daddr;
	fl4->saddr = saddr;
}
				      

struct flowi6 {
	struct flowi_common	__fl_common;
#define flowi6_oif		__fl_common.flowic_oif
#define flowi6_iif		__fl_common.flowic_iif
#define flowi6_mark		__fl_common.flowic_mark
#define flowi6_scope		__fl_common.flowic_scope
#define flowi6_proto		__fl_common.flowic_proto
#define flowi6_flags		__fl_common.flowic_flags
#define flowi6_secid		__fl_common.flowic_secid
#define flowi6_tun_key		__fl_common.flowic_tun_key
#define flowi6_uid		__fl_common.flowic_uid
	struct in6_addr		daddr;
	struct in6_addr		saddr;
	/* Note: flowi6_tos is encoded in flowlabel, too. */
	__be32			flowlabel;
	union flowi_uli		uli;
#define fl6_sport		uli.ports.sport
#define fl6_dport		uli.ports.dport
#define fl6_icmp_type		uli.icmpt.type
#define fl6_icmp_code		uli.icmpt.code
#define fl6_ipsec_spi		uli.spi
#define fl6_mh_type		uli.mht.type
#define fl6_gre_key		uli.gre_key
} __attribute__((__aligned__(BITS_PER_LONG/8)));

struct flowidn {
	struct flowi_common	__fl_common;
#define flowidn_oif		__fl_common.flowic_oif
#define flowidn_iif		__fl_common.flowic_iif
#define flowidn_mark		__fl_common.flowic_mark
#define flowidn_scope		__fl_common.flowic_scope
#define flowidn_proto		__fl_common.flowic_proto
#define flowidn_flags		__fl_common.flowic_flags
	__le16			daddr;
	__le16			saddr;
	union flowi_uli		uli;
#define fld_sport		uli.ports.sport
#define fld_dport		uli.ports.dport
} __attribute__((__aligned__(BITS_PER_LONG/8)));

struct flowi {
	union {
		struct flowi_common	__fl_common;
		struct flowi4		ip4;
		struct flowi6		ip6;
		struct flowidn		dn;
	} u;
#define flowi_oif	u.__fl_common.flowic_oif
#define flowi_iif	u.__fl_common.flowic_iif
#define flowi_mark	u.__fl_common.flowic_mark
#define flowi_tos	u.__fl_common.flowic_tos
#define flowi_scope	u.__fl_common.flowic_scope
#define flowi_proto	u.__fl_common.flowic_proto
#define flowi_flags	u.__fl_common.flowic_flags
#define flowi_secid	u.__fl_common.flowic_secid
#define flowi_tun_key	u.__fl_common.flowic_tun_key
#define flowi_uid	u.__fl_common.flowic_uid
} __attribute__((__aligned__(BITS_PER_LONG/8)));

static inline struct flowi *flowi4_to_flowi(struct flowi4 *fl4)
{
	return container_of(fl4, struct flowi, u.ip4);
}

static inline struct flowi *flowi6_to_flowi(struct flowi6 *fl6)
{
	return container_of(fl6, struct flowi, u.ip6);
}

static inline struct flowi *flowidn_to_flowi(struct flowidn *fldn)
{
	return container_of(fldn, struct flowi, u.dn);
}

typedef unsigned long flow_compare_t;

static inline size_t flow_key_size(u16 family)
{
	switch (family) {
	case AF_INET:
		BUILD_BUG_ON(sizeof(struct flowi4) % sizeof(flow_compare_t));
		return sizeof(struct flowi4) / sizeof(flow_compare_t);
	case AF_INET6:
		BUILD_BUG_ON(sizeof(struct flowi6) % sizeof(flow_compare_t));
		return sizeof(struct flowi6) / sizeof(flow_compare_t);
	case AF_DECnet:
		BUILD_BUG_ON(sizeof(struct flowidn) % sizeof(flow_compare_t));
		return sizeof(struct flowidn) / sizeof(flow_compare_t);
	}
	return 0;
}

#define FLOW_DIR_IN	0
#define FLOW_DIR_OUT	1
#define FLOW_DIR_FWD	2

struct net;
struct sock;
struct flow_cache_ops;

struct flow_cache_object {
	const struct flow_cache_ops *ops;
};

struct flow_cache_ops {
	struct flow_cache_object *(*get)(struct flow_cache_object *);
	int (*check)(struct flow_cache_object *);
	void (*delete)(struct flow_cache_object *);
};

typedef struct flow_cache_object *(*flow_resolve_t)(
		struct net *net, const struct flowi *key, u16 family,
		u8 dir, struct flow_cache_object *oldobj, void *ctx);

struct flow_cache_object *flow_cache_lookup(struct net *net,
					    const struct flowi *key, u16 family,
					    u8 dir, flow_resolve_t resolver,
					    void *ctx);
int flow_cache_init(struct net *net);
void flow_cache_fini(struct net *net);

void flow_cache_flush(struct net *net);
void flow_cache_flush_deferred(struct net *net);
extern atomic_t flow_cache_genid;

__u32 __get_hash_from_flowi6(const struct flowi6 *fl6, struct flow_keys *keys);

static inline __u32 get_hash_from_flowi6(const struct flowi6 *fl6)
{
	struct flow_keys keys;

	return __get_hash_from_flowi6(fl6, &keys);
}

__u32 __get_hash_from_flowi4(const struct flowi4 *fl4, struct flow_keys *keys);

static inline __u32 get_hash_from_flowi4(const struct flowi4 *fl4)
{
	struct flow_keys keys;

	return __get_hash_from_flowi4(fl4, &keys);
}

#endif
