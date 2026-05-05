/*
 * Copyright (c) 2022-2025, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/if.h>
#include <linux/rculist.h>
#include <linux/netdevice.h>
#include <linux/rculist.h>
#include <net/addrconf.h>
#include <net/xfrm.h>
#include <crypto/aes.h>
#include <ppe_acl.h>

#include "eip_ipsec_priv.h"

/*
 * eip_ipsec_sa_data_to_tr_info()
 *	Fill TR infor using SA data passed by user.
 */
static void eip_ipsec_sa_data_to_tr_info(struct eip_ipsec_data *data, struct eip_ipsec_tuple *t, struct eip_tr_info *tr_info)
{
	struct eip_tr_info_ipsec *ipsec = &tr_info->ipsec;
	struct eip_tr_base *base = &tr_info->base;
	uint32_t flags = data->base.flags;

	memset(tr_info, 0, sizeof(*tr_info));

	base->svc = eip_is_inline_supported() ? EIP_SVC_HYBRID_IPSEC : EIP_SVC_IPSEC;

	strscpy(base->alg_name, data->base.algo_name, CRYPTO_MAX_ALG_NAME);
	base->cipher.key_data = data->base.cipher.data;
	base->cipher.key_len = data->base.cipher.len;
	base->auth.key_data = data->base.auth.data;
	base->auth.key_len = data->base.auth.len;
	base->nonce = data->base.nonce;

	ipsec->icv_len = data->base.icv_len;
	ipsec->sa_flags |= (flags & EIP_IPSEC_FLAG_ENC) ? EIP_TR_IPSEC_FLAG_ENC : 0;
	ipsec->sa_flags |= (flags & EIP_IPSEC_FLAG_IPV6) ? EIP_TR_IPSEC_FLAG_IPV6 : 0;
	ipsec->sa_flags |= (flags & EIP_IPSEC_FLAG_UDP) ? EIP_TR_IPSEC_FLAG_UDP : 0;
	ipsec->sa_flags |= (flags & EIP_IPSEC_FLAG_ESN) ? EIP_TR_IPSEC_FLAG_ESN : 0;
	ipsec->sa_flags |= (flags & EIP_IPSEC_FLAG_TUNNEL) ? EIP_TR_IPSEC_FLAG_TUNNEL : 0;
	ipsec->sa_flags |= (flags & EIP_IPSEC_FLAG_SKIP) ? EIP_TR_IPSEC_FLAG_SKIP : 0;
	ipsec->sa_flags |= (flags & EIP_IPSEC_FLAG_CP_TOS) ? EIP_TR_IPSEC_FLAG_CP_TOS : 0;
	ipsec->sa_flags |= (flags & EIP_IPSEC_FLAG_CP_DF) ? EIP_TR_IPSEC_FLAG_CP_DF : 0;
	ipsec->ip_ttl = data->hop_limit;
	ipsec->ip_df = (flags & EIP_IPSEC_FLAG_CP_DF) ? 0 : data->df;
	ipsec->ip_dscp = (flags & EIP_IPSEC_FLAG_CP_TOS) ? 0 : data->dscp;

	switch (data->replay_win) {
	case 0:
		ipsec->replay = EIP_IPSEC_REPLAY_NONE;
		ipsec->sa_flags &= ~EIP_IPSEC_FLAG_ESN;
		break;
	case 1 ... 32:
		ipsec->replay = EIP_IPSEC_REPLAY_32;
		break;
	case 33 ... 64:
		ipsec->replay = EIP_IPSEC_REPLAY_64;
		break;
	case 65 ... 128:
		ipsec->replay = EIP_IPSEC_REPLAY_128;
		break;
	case 129 ... 256:
		ipsec->replay = EIP_IPSEC_REPLAY_256;
		break;
	default:
		ipsec->replay = EIP_IPSEC_REPLAY_384;
		break;
	}

	ipsec->spi_idx = htonl(t->esp_spi);
	ipsec->src_port = htons(t->sport);
	ipsec->dst_port = htons(t->dport);

	if (t->ip_version == 4) {
		ipsec->ip_ver = 4;
		ipsec->src_ip[0] = htonl(t->src_ip[0]);
		ipsec->dst_ip[0] = htonl(t->dest_ip[0]);
	} else {
		ipsec->ip_ver = 6;
		ipsec->src_ip[0] = htonl(t->src_ip[0]);
		ipsec->src_ip[1] = htonl(t->src_ip[1]);
		ipsec->src_ip[2] = htonl(t->src_ip[2]);
		ipsec->src_ip[3] = htonl(t->src_ip[3]);
		ipsec->dst_ip[0] = htonl(t->dest_ip[0]);
		ipsec->dst_ip[1] = htonl(t->dest_ip[1]);
		ipsec->dst_ip[2] = htonl(t->dest_ip[2]);
		ipsec->dst_ip[3] = htonl(t->dest_ip[3]);
	}
}

/*
 * eip_ipsec_sa_match_addr()
 *	Match SA destination IP address.
 */
static bool eip_ipsec_sa_match_addr(struct eip_ipsec_sa *sa, uint8_t version, __be32 *addr)
{
	uint32_t status = 0;

	status += sa->tuple.ip_version ^ version;

	if (version == 4) {
		status += sa->tuple.dest_ip[0] ^ addr[0];
		return !status;
	} else {
		status += sa->tuple.dest_ip[0] ^ addr[0];
		status += sa->tuple.dest_ip[1] ^ addr[1];
		status += sa->tuple.dest_ip[2] ^ addr[2];
		status += sa->tuple.dest_ip[3] ^ addr[3];
		return !status;
	}
}

/*
 * eip_ipsec_sa_match_spi()
 *	Lookup SA using SPI index in given database.
 *
 * This function must be called with rcu_read_lock_bh() or spin_lock_bh().
 */
struct eip_ipsec_sa *eip_ipsec_sa_match_spi(struct list_head *sa_head, __be32 spi)
{
	struct eip_ipsec_sa *sa;

	list_for_each_entry_rcu(sa, sa_head, node) {
		if (likely(sa->spi == spi)) {
			return sa;
		}
	}

	return NULL;
}

/*
 * eip_ipsec_sa_dettach_rcu()
 *	Called at RCU qs state to deref SA object.
 */
static void eip_ipsec_sa_dettach_rcu(struct rcu_head *head)
{
	struct eip_ipsec_sa *sa = container_of(head, struct eip_ipsec_sa, rcu);
	eip_ipsec_sa_deref(sa);
}

/*
 * eip_ipsec_sa_hash_v4()
 */
static inline uint32_t eip_ipsec_sa_hash_v4(__be32 *addr, __be32 spi)
{
	uint32_t hash;

	hash = addr[0];
	hash ^= spi ^ 4;

	return ((hash >> EIP_IPSEC_HASH_SHIFT) ^ hash) & EIP_IPSEC_HASH_MASK;
}

/*
 * eip_ipsec_sa_hash_v6()
 */
static inline uint32_t eip_ipsec_sa_hash_v6(__be32 *addr, __be32 spi)
{
	uint32_t hash;

	hash = addr[0] ^ addr[1] ^ addr[2] ^ addr[3];
	hash ^= spi ^ 6;

	return ((hash >> EIP_IPSEC_HASH_SHIFT) ^ hash) & EIP_IPSEC_HASH_MASK;
}

/*
 * eip_ipsec_sa_attach_global()
 *	Attach IPsec SA to global hashlist.
 */
static void eip_ipsec_sa_attach_global(struct eip_ipsec_sa *sa)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	uint32_t hash;

	lockdep_assert_held(&drv->lock);

	if (sa->ip_ver == 4) {
		hash = eip_ipsec_sa_hash_v4(sa->dst_ip, sa->spi);
	} else {
		hash = eip_ipsec_sa_hash_v6(sa->dst_ip, sa->spi);
	}

	hlist_add_head_rcu(&sa->hnode, &drv->sa_hlist[hash]);
}

/*
 * eip_ipsec_sa_dettach_global()
 *	Dettach IPsec SA from global hashlist.
 */
static void eip_ipsec_sa_dettach_global(struct eip_ipsec_sa *sa)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	lockdep_assert_held(&drv->lock);
	hlist_del_init_rcu(&sa->hnode);
}

/*
 * eip_ipsec_sa_attach()
 *	Attach SA into device encap/decap DB.
 */
static void eip_ipsec_sa_attach(struct net_device *ndev, struct eip_ipsec_sa *sa, bool encap)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	struct eip_ipsec_dev *eid = netdev_priv(ndev);
	struct list_head *sa_head = &eid->enc_sa;
	struct eip_ipsec_dev_stats *dev_stats;

	dev_stats = this_cpu_ptr(eid->stats_pcpu);

	/*
	 * Attach SA to corresponding per device encap or decap list.
	 * Also, attach decap SA to global list for received side lookup.
	 */
	spin_lock_bh(&drv->lock);

	if (!encap) {
		eip_ipsec_sa_attach_global(sa);
		sa_head = &eid->dec_sa;
	}

	list_add_rcu(&sa->node, sa_head);
	dev_stats->sa_added++;
	spin_unlock_bh(&drv->lock);
}

/*
 * __sa_lookup()
 *	Lookup SA using tuple.
 */
static struct eip_ipsec_sa *__sa_lookup(struct net_device *ndev, struct eip_ipsec_tuple *t)
{
	struct eip_ipsec_dev *eid = netdev_priv(ndev);
	struct list_head *sa_head;
	struct eip_ipsec_sa *sa;

	/*
	 * Lookup into Encap SA list.
	 */
	sa_head = &eid->enc_sa;
	sa = eip_ipsec_sa_match_spi(sa_head, htonl(t->esp_spi));
	if (sa && eip_ipsec_sa_match_addr(sa, t->ip_version, t->dest_ip)) {
		pr_debug("%px: SA is present in encap device database", ndev);
		return sa;
	}

	/*
	 * Lookup into Decap SA list.
	 */
	sa_head = &eid->dec_sa;
	sa = eip_ipsec_sa_match_spi(sa_head, htonl(t->esp_spi));
	if (!sa || !eip_ipsec_sa_match_addr(sa, t->ip_version, t->dest_ip)) {
		pr_debug("%px: SA is not present in decap device database", ndev);
		return NULL;
	}

	return sa;
}

/*
 * eip_ipsec_sa_ref_get
 *	Lookup SA using tuple and take ref.
 */
struct eip_ipsec_sa *eip_ipsec_sa_ref_get(struct net_device *ndev, struct eip_ipsec_tuple *t)
{
	struct eip_ipsec_sa *sa;

	rcu_read_lock_bh();

	sa = __sa_lookup(ndev, t);
	if (likely(sa))
		eip_ipsec_sa_ref(sa);

	rcu_read_unlock_bh();
	return sa;
}

/*
 * eip_ipsec_sa_dettach()
 *	Dettach SA from device encap/decap DB.
 */
static struct eip_ipsec_sa *eip_ipsec_sa_dettach(struct net_device *ndev, struct eip_ipsec_tuple *t)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	struct eip_ipsec_dev *eid = netdev_priv(ndev);
	struct eip_ipsec_dev_stats *dev_stats;
	struct eip_ipsec_sa *sa;

	dev_stats = this_cpu_ptr(eid->stats_pcpu);
	spin_lock_bh(&drv->lock);

	sa = __sa_lookup(ndev, t);
	if (!sa) {
		spin_unlock_bh(&drv->lock);
		return NULL;
	}

	list_del_rcu(&sa->node);
	eip_ipsec_sa_dettach_global(sa);
	dev_stats->sa_removed++;
	spin_unlock_bh(&drv->lock);

	/*
	 * Reference: eip_ipsec_sa_add()
	 * This can be final deref. Schedule it after RCU grace period.
	 */
	call_rcu(&sa->rcu, eip_ipsec_sa_dettach_rcu);
	return sa;
}

/*
 * eip_ipsec_sa_exist()
 *	Check if SA with SPI index exist in encap/decap database.
 *
 */
static bool eip_ipsec_sa_exist(struct net_device *ndev, __be32 spi, bool encap)
{
	struct eip_ipsec_dev *eid = netdev_priv(ndev);
	struct list_head *sa_head = &eid->enc_sa;
	bool exist = false;

	if (!encap) {
		sa_head = &eid->dec_sa;
	}

	/*
	 * API can be called from process context.
	 */
	rcu_read_lock_bh();

	if (eip_ipsec_sa_match_spi(sa_head, spi)) {
		exist = true;
	}

	rcu_read_unlock_bh();
	return exist;
}

#if defined(EIP_IPSEC_VP_SUPP)

/*
 * eip_ipsec_ppe_mdata_set()
 *	Set PPE meta data for TR.
 */
static void eip_ipsec_ppe_mdata_set(struct eip_tr_info *tr_info, ppe_vp_num_t vp_num)
{
	struct eip_ipsec_ppe_mdata *mdata = (struct eip_ipsec_ppe_mdata *)tr_info->ipsec.ppe_mdata;
	memset(mdata, 0, sizeof(*mdata));

	mdata->ppe_src_port = (0x2000 | vp_num);
	mdata->service_code = PPE_DRV_SC_IPSEC_EIP2PPE;
	mdata->fake_mac = true;
	mdata->fake_l2_prot = true;
}

#endif

/*
 * eip_ipsec_sa_set_mtu()
 *      Set IPsec device MTU.
 */
static void eip_ipsec_sa_set_mtu(struct eip_ipsec_sa *sa, struct eip_ipsec_data *sa_data)
{
	struct eip_ipsec_base *base = &sa_data->base;
	struct net_device *ndev = sa->ndev;
	struct eip_tr_algo_info algo = {0};
	struct eip_tr *tr = sa->tr;
	uint32_t mtu_overhead = 0;
	uint32_t mode;

	mtu_overhead = sizeof(struct ip_esp_hdr);
	mode = base->flags & (EIP_IPSEC_FLAG_TUNNEL | EIP_IPSEC_FLAG_UDP | EIP_IPSEC_FLAG_IPV6);

	switch (mode) {
	case EIP_IPSEC_FLAG_TUNNEL:
		mtu_overhead += sizeof(struct iphdr);
		break;
	case (EIP_IPSEC_FLAG_TUNNEL | EIP_IPSEC_FLAG_IPV6):
		mtu_overhead += sizeof(struct ipv6hdr);
		break;
	case (EIP_IPSEC_FLAG_TUNNEL | EIP_IPSEC_FLAG_UDP):
		mtu_overhead += (sizeof(struct iphdr) + sizeof(struct udphdr));
		break;
	case EIP_IPSEC_FLAG_UDP:
		mtu_overhead += sizeof(struct udphdr);
		break;
	case EIP_IPSEC_FLAG_IPV6:
	default:
		break;
	}

	eip_tr_get_algo_info(tr, &algo);
	mtu_overhead += (algo.blk_len + algo.iv_len + base->icv_len);

	rtnl_lock();
	dev_set_mtu(ndev, sa_data->mtu - mtu_overhead);
	rtnl_unlock();
}

/*
 * eip_ipsec_sa_stats_sync
 *	sync and store dev and SA stats from TR.
 */
void eip_ipsec_sa_stats_sync(struct net_device *ndev, struct eip_ipsec_sa *sa)
{
	struct eip_ipsec_dev_stats *dev_stats;
	struct eip_ipsec_dev *eid = netdev_priv(ndev);
	struct eip_hw_stats delta_stats = {0};
	struct eip_ipsec_sa_stats *sa_stats;

	dev_stats = this_cpu_ptr(eid->stats_pcpu);
	sa_stats = this_cpu_ptr(sa->stats_pcpu);

	/*  Fetch stats from Driver */
	eip_tr_get_hw_stats(sa->tr, &delta_stats );

	if (sa->flags & EIP_IPSEC_FLAG_ENC) {
		dev_stats->tx_pkts +=  delta_stats.pkt_cnt;
		dev_stats->tx_bytes += delta_stats.byte_cnt;
		sa_stats->tx_pkts += delta_stats.pkt_cnt;
		sa_stats->tx_bytes += delta_stats.byte_cnt;
	} else {

		dev_stats->rx_pkts += delta_stats.pkt_cnt;
		dev_stats->rx_bytes += delta_stats.byte_cnt;
		sa_stats->rx_pkts += delta_stats.pkt_cnt;
		sa_stats->rx_bytes += delta_stats.byte_cnt;
	}
}

/*
 * eip_ipsec_dev_get_summary_stats()
 *	Update the summary stats.
 */
static void eip_ipsec_sa_stats_get(struct eip_ipsec_sa *sa,
		struct eip_ipsec_sa_stats *stats)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	int words;
	int cpu;
	int i;

	if (eip_feature_check(drv->ctx, EIP_OFFLOAD_INNER_FLOW) &&
			eip_feature_check(drv->ctx, EIP_OFFLOAD_OUTER_FLOW)) {
		eip_ipsec_sa_stats_sync(sa->ndev, sa);
	}

	words = (sizeof(*stats) / sizeof(uint64_t));
	memset(stats, 0, sizeof(*stats));

	/*
	 * All statistics are 64bit. So we can just iterate by words.
	 */
	for_each_possible_cpu(cpu) {
		const struct eip_ipsec_sa_stats *sp = per_cpu_ptr(sa->stats_pcpu, cpu);
		uint64_t *stats_ptr = (uint64_t *)stats;
		uint64_t *sp_ptr = (uint64_t *)sp;

		for (i = 0; i < words; i++, stats_ptr++, sp_ptr++)
			*stats_ptr += *sp_ptr;
	}
}

/*
 * eip_ipsec_sa_read()
 *	Read the all SA statistics in provided buffer.
 */
ssize_t eip_ipsec_sa_read(struct net_device *ndev, bool encap, char *buf, ssize_t max_len)
{
	struct eip_ipsec_dev *eid = netdev_priv(ndev);
	struct list_head *sa_head = &eid->enc_sa;
	char *hdr = "Encap";
	struct eip_ipsec_sa_stats stats;
	struct eip_ipsec_sa *sa;
	ssize_t len = 0;
	int i;

	/*
	 * Encapsulation SAs
	 */
	if (!encap) {
		sa_head = &eid->dec_sa;
		hdr = "Decap";
	}

	rcu_read_lock_bh();

	i = 0;
	list_for_each_entry_rcu(sa, sa_head, node) {
		struct eip_ipsec_tuple *t = &sa->tuple;
		eip_ipsec_sa_stats_get(sa, &stats);

		if (t->ip_version == IPVERSION) {
			len += snprintf(buf + len, max_len - len, "%s SA%u (src:%pI4h dst:%pI4h spi:0x%X sport:%u dport:%u proto:%u):\n",
					hdr, ++i, t->src_ip, t->dest_ip, t->esp_spi, t->sport, t->dport, t->protocol);
		} else {
			len += snprintf(buf + len, max_len - len, "%s SA%u (src:%pI6 dst:%pI6 spi:0x%X sport:%u dport:%u proto:%u):\n",
					hdr, ++i, t->src_ip, t->dest_ip, t->esp_spi, t->sport, t->dport, t->protocol);

		}
		len += snprintf(buf + len, max_len - len, "\tActive refs: %u\n", kref_read(&sa->ref));
		len += snprintf(buf + len, max_len - len, "\tTx packets: %llu\n", stats.tx_pkts);
		len += snprintf(buf + len, max_len - len, "\tTx bytes: %llu\n", stats.tx_bytes);
		len += snprintf(buf + len, max_len - len, "\tRx packets: %llu\n", stats.rx_pkts);
		len += snprintf(buf + len, max_len - len, "\tRx bytes: %llu\n", stats.rx_bytes);
		len += snprintf(buf + len, max_len - len, "\tRoute cache miss: %llu\n", stats.dst_cache_miss);
		len += snprintf(buf + len, max_len - len, "\tExpand error: %llu\n", stats.fail_expand);
		len += snprintf(buf + len, max_len - len, "\tEnqueue error: %llu\n", stats.fail_enqueue);
		len += snprintf(buf + len, max_len - len, "\tTransformation error: %llu\n", stats.fail_transform);
		len += snprintf(buf + len, max_len - len, "\tRoute error: %llu\n", stats.fail_route);
		len += snprintf(buf + len, max_len - len, "\tSP allocation error: %llu\n", stats.fail_sp_alloc);
		len += snprintf(buf + len, max_len - len, "\tFast recv miss: %llu\n", stats.fast_recv_miss);
	}

	rcu_read_unlock_bh();

	return len;
}

/*
 * eip_ipsec_sa_ref_get_encap()
 *	Return head SA from encap DB.
 */
struct eip_ipsec_sa *eip_ipsec_sa_ref_get_encap(struct net_device *ndev)
{
	struct eip_ipsec_dev *eid = netdev_priv(ndev);
	struct eip_ipsec_sa *sa;

	rcu_read_lock_bh();

	sa = list_first_or_null_rcu(&eid->enc_sa, struct eip_ipsec_sa, node);
	if (likely(sa))
		eip_ipsec_sa_ref(sa);

	rcu_read_unlock_bh();
	return sa;
}

/*
 * eip_ipsec_sa_ref_get_decap()
 *	Return head decap SA.
 */
struct eip_ipsec_sa *eip_ipsec_sa_ref_get_decap(struct net_device *ndev)
{
	struct eip_ipsec_dev *eid = netdev_priv(ndev);
	struct eip_ipsec_sa *sa;

	rcu_read_lock_bh();

	sa = list_first_or_null_rcu(&eid->dec_sa, struct eip_ipsec_sa, node);
	if (likely(sa))
		eip_ipsec_sa_ref(sa);

	rcu_read_unlock_bh();
	return sa;
}

/*
 * eip_ipsec_sa_ref_get_decap_v6()
 *	Lookup SA using IPv6 address and SPI.
 */
struct eip_ipsec_sa *eip_ipsec_sa_ref_get_decap_v6(__be32 *addr, __be32 spi)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	uint32_t hash = eip_ipsec_sa_hash_v6(addr, spi);
	struct eip_ipsec_sa *sa;

	rcu_read_lock_bh();

	hlist_for_each_entry_rcu(sa, &drv->sa_hlist[hash], hnode) {
		if (ipv6_addr_equal((struct in6_addr *)sa->dst_ip, (struct in6_addr *)addr)
				&&  (sa->spi == spi) && (sa->ip_ver == 6)) {
			eip_ipsec_sa_ref(sa);

			rcu_read_unlock_bh();
			return sa;
		}
	}

	rcu_read_unlock_bh();
	return NULL;
}

/*
 * eip_ipsec_sa_ref_get_decap_v4()
 *	Lookup SA using IPv4 address and SPI.
 */
struct eip_ipsec_sa *eip_ipsec_sa_ref_get_decap_v4(__be32 *addr, __be32 spi)
{
	uint32_t hash = eip_ipsec_sa_hash_v4(addr, spi);
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	struct eip_ipsec_sa *sa;

	rcu_read_lock_bh();

	hlist_for_each_entry_rcu(sa, &drv->sa_hlist[hash], hnode) {
		if ((sa->dst_ip[0] == addr[0]) &&  (sa->spi == spi) && (sa->ip_ver == 4)) {
			eip_ipsec_sa_ref(sa);

			rcu_read_unlock_bh();
			return sa;
		}
	}

	rcu_read_unlock_bh();
	return NULL;
}

/*
 * eip_ipsec_sa_final()
 *	Final ref released for SA.
 */
void eip_ipsec_sa_final(struct kref *kref)
{
	struct eip_ipsec_sa *sa = container_of(kref, struct eip_ipsec_sa, ref);

	complete(&sa->completion);
}

/*
 * eip_ipsec_sa_add()
 *	Create a IPsec SA under netdevice.
 *
 * Caller needs to ensure netdevice is not being destroyed while calling this API.
 */
int eip_ipsec_sa_add(struct net_device *ndev, struct eip_ipsec_data *data, struct eip_ipsec_tuple *t)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	struct eip_ipsec_dev *eid = netdev_priv(ndev);
	struct eip_ipsec_sa *old_sa = NULL;
	struct eip_tr_info tr_info;
	struct eip_ipsec_sa *sa;
	bool udp_encap;
	bool encap;
	int err;

	encap = (data->base.flags & EIP_IPSEC_FLAG_ENC);
	udp_encap = (data->base.flags & EIP_IPSEC_FLAG_UDP);

	pr_debug("%p: algo_name %s, icv_len %u, flags %u replay_win %u df %u dscp %u hop_limit %u\n",
		ndev, data->base.algo_name, data->base.icv_len, data->base.flags, data->replay_win, data->df,
		data->dscp, data->hop_limit);

	if (t->ip_version == IPVERSION) {
		pr_debug("%p: src:%pI4h dst:%pI4h spi:0x%X sport:%u dport:%u proto:%u\n",
				ndev, t->src_ip, t->dest_ip, t->esp_spi, t->sport, t->dport, t->protocol);
	} else {
		pr_debug("%p: src:%pI6 dst:%pI6 spi:0x%X sport:%u dport:%u proto:%u\n",
				ndev, t->src_ip, t->dest_ip, t->esp_spi, t->sport, t->dport, t->protocol);
	}

	/*
	 * Sanity checks:
	 * 	Algorithm is supported by hardware.
	 * 	SPI index has to be unique for database.
	 */
	if (!eip_ctx_algo_supported(drv->ctx, data->base.algo_name)) {
		pr_warn("%px: SA algorithm not supported(%s)\n", ndev, data->base.algo_name);
		return -EINVAL;
	}

	if (eip_ipsec_sa_exist(ndev, htonl(t->esp_spi), encap)) {
		pr_warn("%px: Duplicate SA add for SPI(%u)\n", ndev, t->esp_spi);
		return -EEXIST;
	}

	sa = kzalloc(sizeof(*sa), GFP_KERNEL);
	if (!sa) {
		pr_warn("%px: Failed to allocate SA\n", ndev);
		return -ENOMEM;
	}

	sa->stats_pcpu = alloc_percpu_gfp(struct eip_ipsec_sa_stats, GFP_KERNEL | __GFP_ZERO);
	if (!sa->stats_pcpu) {
		pr_err("%px: Failed to allocate stats memory for SA\n", ndev);
		err = -ENOMEM;
		goto fail_pcpu;
	}

	err = dst_cache_init(&sa->dst_cache, GFP_KERNEL);
	if (err) {
		pr_err("%px: Failed to initialize dst for SA\n", ndev);
		goto fail_dst;
	}

	/*
	 * Dereference: eip_ipsec_sa_final()
	 */
	dev_hold(ndev);
	sa->ndev = ndev;
	sa->eid = eid;

	/*
	 * Dereference: eip_ipsec_sa_dettach_rcu()
	 */
	kref_init(&sa->ref);
	INIT_LIST_HEAD(&sa->node);
	INIT_HLIST_NODE(&sa->hnode);
	init_completion(&sa->completion);
	sa->head_room = EIP_IPSEC_DEV_MAX_HEADROOM; /* TODO: Calculate using SA type */
	sa->tail_room = EIP_IPSEC_DEV_MAX_TAILROOM;
	sa->tuple = *t;
	sa->flags = data->base.flags;

	/*
	 * Cache part of tuple used in datapath in network order.
	 */
	sa->spi = htonl(t->esp_spi);
	if (t->ip_version == 4) {
		sa->ip_ver = 4;
		sa->dst_ip[0] = htonl(t->dest_ip[0]);
	} else {
		sa->ip_ver = 6;
		sa->dst_ip[0] = htonl(t->dest_ip[0]);
		sa->dst_ip[1] = htonl(t->dest_ip[1]);
		sa->dst_ip[2] = htonl(t->dest_ip[2]);
		sa->dst_ip[3] = htonl(t->dest_ip[3]);
	}

	eip_ipsec_sa_data_to_tr_info(data, t, &tr_info);

	/*
	 * Set completion handler.
	 */
	tr_info.ipsec.app_data = sa;
	if (encap) {
		tr_info.ipsec.cb = (t->ip_version == 4) ? eip_ipsec_dev_enc_done_v4 : eip_ipsec_dev_enc_done_v6;
		tr_info.ipsec.err_cb = eip_ipsec_dev_enc_err;
	} else {
		tr_info.ipsec.cb = eip_ipsec_proto_dec_done;
		tr_info.ipsec.err_cb = eip_ipsec_proto_dec_err;
	}

	/*
	 * Override enc callback and fill PPE metadata for hybrid inline channel.
	 */
#if defined(EIP_IPSEC_VP_SUPP)
	if (encap) {
		tr_info.ipsec.cb = eip_ipsec_dev_enc_done_hy;
	}

	eip_ipsec_ppe_mdata_set(&tr_info, eid->vp_num);
#endif

	sa->tr = eip_tr_alloc(drv->ctx, &tr_info);
	if (!sa->tr) {
		pr_warn("%px: Failed to allocate HW record\n", ndev);
		err = -EINVAL;
		goto fail_tr;
	}

	/*
	 * If its a rekey for encap SA, Update the tunnel with new TR
	 */
	if (encap && eip_feature_check(drv->ctx, EIP_OFFLOAD_INNER_FLOW)) {
		old_sa = eip_ipsec_sa_ref_get_encap(ndev);
	}

	sa->xs = data->xs;

	if (!encap && udp_encap) {
		eip_ipsec_proto_udp_sock_override(t);
	}

	/*
	 * Update IPsec net device MTU
	 */
	if (encap) {
		eip_ipsec_sa_set_mtu(sa, data);
	}

	/*
	 * Set ACL rule id to invalid
	 */
	sa->acl_rule_id = EIP_IPSEC_INVALID_ACL_RULE_ID;

	/*
	 * TODO: Do we need to handle multiple sa_add() on multiple core.
	 */
	eip_ipsec_sa_attach(ndev, sa, encap);

	if (old_sa) {
		struct eip_tun_update_info info = {0};

		info.valid_flags |= EIP_TUN_TR_VALID;
		info.tr = old_sa->tr;
		info.new_tr = sa->tr;

		eip_tun_update(eid->tun, &info);
		eip_ipsec_sa_deref(old_sa);
	}

	/*
	 * If flow is decap direction add flow to EIP.
	 */
	if (!encap) {
		if (eip_feature_check(drv->ctx, EIP_OFFLOAD_OUTER_FLOW)) {
			if (eip_ipsec_flow_add(ndev, &sa->tuple, sa)) {
				/*
				 * If flow addition fails, fall back to the slow path so that
				 * packets continue to be processed without offload.
				 */
				pr_warn("%px: Failed to add flow for inline, Throughput will be impacted\n", ndev);
				goto out;
			}
		}

		if (!eip_feature_check(drv->ctx, EIP_OFFLOAD_INNER_FLOW))
			return 0;

		/*
		 * When both inner and outer offload are supported, the flow hardware
		 * operates in full inline mode. In this case, configure ACL rules.
		 */
		if (eip_feature_check(drv->ctx, EIP_OFFLOAD_INNER_FLOW) && eip_feature_check(drv->ctx, EIP_OFFLOAD_OUTER_FLOW)) {
			sa->acl_rule_id = eip_ipsec_flow_match_spi(ntohl(sa->spi), ndev, sa->flags);
			if (sa->acl_rule_id == EIP_IPSEC_INVALID_ACL_RULE_ID) {
				pr_debug("%px: Failed to create ACL rule\n", sa);
				goto acl_fail;
			}
		}
	}

out:
	return 0;

acl_fail:
	eip_ipsec_flow_del(ndev,  &sa->tuple, encap);
fail_tr:
	dev_put(ndev);
	dst_cache_destroy(&sa->dst_cache);
fail_dst:
	free_percpu(sa->stats_pcpu);
fail_pcpu:
	kfree(sa);
	return err;
}

/*
 * eip_ipsec_sa_del()
 *	Delete IPsec SA associated with the netdevice.
 */
void eip_ipsec_sa_del(struct net_device *ndev, struct eip_ipsec_tuple *t)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	struct eip_ipsec_sa *sa;
	s16 acl_ret;

	might_sleep();

	/*
	 * Dettach and deref the SA. This also ensure no new packet gets reference.
	 */
	sa = eip_ipsec_sa_dettach(ndev, t);
	if (!sa) {
		pr_warn("%px: SA not found\n", ndev);
		return;
	}

	if (sa->flags & EIP_IPSEC_FLAG_ENC)
		goto tr_disable;

	/*
	 * If flow is decap direction adelete flow to EIP.
	 */
	if (eip_feature_check(drv->ctx, EIP_OFFLOAD_OUTER_FLOW)) {
		/*
		 * Clean up the associated ACL rule when a valid rule ID is present.
		 * Note: ACL rules are configured and valid only when the hardware
		 * is operating in full inline mode.
		 */
		if (sa->acl_rule_id != EIP_IPSEC_INVALID_ACL_RULE_ID) {
			acl_ret = ppe_acl_rule_destroy(sa->acl_rule_id);
			if (acl_ret != PPE_ACL_RET_SUCCESS) {
				pr_warn("%px: Failed to destroy ACL rule ID %d, error %u\n",
						sa, sa->acl_rule_id, acl_ret);
			}
		}

		/*
		 * Delete Flow.
		 */
		eip_ipsec_flow_del(ndev, &sa->tuple, false);
	}

tr_disable:
	/*
	 * Disable TR. This also ensure no new packet being sent by driver.
	 */
	eip_tr_disable(sa->tr);

	/*
	 * Wait for all outstanding SA operation.
	 */
	wait_for_completion(&sa->completion);

	/*
	 * Just before rekease TR get final stats
	 */
	if (eip_feature_check(drv->ctx, EIP_OFFLOAD_INNER_FLOW) &&
			eip_feature_check(drv->ctx, EIP_OFFLOAD_OUTER_FLOW)) {
		eip_ipsec_sa_stats_sync(ndev, sa);
	}

	/*
	 * Release the TR and other SA resources
	 */
	eip_tr_free(sa->tr);
	sa->xs = NULL;
	dev_put(sa->ndev);
	dst_cache_destroy(&sa->dst_cache);
	free_percpu(sa->stats_pcpu);
	kfree(sa);
}
