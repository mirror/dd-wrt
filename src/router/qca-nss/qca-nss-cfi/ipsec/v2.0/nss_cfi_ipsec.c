/* Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 */

/* nss_cfi_ipsec.c
 *	NSS IPsec offload glue for Openswan/KLIPS
 */
#include <linux/version.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/memory.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/if.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <crypto/aes.h>
#include <crypto/des.h>
#include <net/esp.h>

#include <nss_api_if.h>
#include <nss_ipsec.h>
#include <nss_cfi_if.h>
#include <nss_ipsecmgr.h>

#define NSS_CFI_IPSEC_BASE_NAME "ipsec"
#define NSS_CFI_IPSEC_TUNNEL_MAX 8
#define NSS_CFI_IPSEC_SES_MASK 0xffff

/*
 * This is used by KLIPS for communicate the device along with the
 * packet. We need this to derive the mapping of the incoming flow
 * to the IPsec tunnel
 */
struct nss_cfi_ipsec_skb_cb {
	struct net_device *hlos_dev;
	bool natt;
};

/*
 * Per tunnel object created w.r.t the HLOS IPsec stack
 */
struct nss_cfi_ipsec_sa {
	struct list_head list;
	uint32_t sid;
	struct nss_ipsecmgr_flow_outer outer;
};

/*
 * CFI IPsec netdevice  mapping between HLOS devices and NSS devices
 * Essentially various HLOS IPsec devices will be used as indexes to
 * map into the NSS devices. For example
 * "ipsec0" --> "ipsectun0"
 * "ipsec1" --> "ipsectun1"
 *
 * Where the numeric suffix of "ipsec0", "ipsec1" is used to index into
 * the table
 */
struct nss_cfi_ipsec_tunnel_entry {
	int32_t klips_dev_index;
	int32_t nss_dev_index;
	struct list_head sa_list;
};

/*
 * NSS IPsec CFI tunnel map table
 */
struct nss_cfi_ipsec_tunnel {
	uint16_t max;
	uint16_t used;
	rwlock_t lock;
	struct nss_cfi_ipsec_tunnel_entry *tbl;
};

static int tunnel_max = NSS_CFI_IPSEC_TUNNEL_MAX;
module_param(tunnel_max, int, 0644);
MODULE_PARM_DESC(tunnel_max, "Maximum number of tunnels to offload");

static struct nss_cfi_ipsec_tunnel tunnel_map;	/* per tunnel device table */

/*
 * nss_cfi_ipsec_v6addr_ntoh()
 *	Network to host order
 */
static void nss_cfi_ipsec_v6addr_ntoh(uint32_t *dest, uint32_t *src)
{
	dest[0] = ntohl(src[0]);
	dest[1] = ntohl(src[1]);
	dest[2] = ntohl(src[2]);
	dest[3] = ntohl(src[3]);
}

/*
 * nss_cfi_ipsec_get_iv_blk_len
 *	get ipsec algorithm specific iv and block len
 */
static int32_t nss_cfi_ipsec_get_iv_blk_len(enum nss_crypto_cmn_algo algo)
{
	switch (algo) {
	case NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC:
	case NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA160_HMAC:
	case NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA160_HMAC:
		return AES_BLOCK_SIZE;

	case NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA160_HMAC:
		return DES_BLOCK_SIZE;

	case NSS_CRYPTO_CMN_ALGO_NULL:
		return 0;

	default:
		nss_cfi_err("Invalid algorithm\n");
		return -1;
	}
}

/*
 * nss_cfi_ipsec_get_algo
 *	get ipsec manager specfic algorithm
 */
static enum nss_ipsecmgr_algo nss_cfi_ipsec_get_algo(enum nss_crypto_cmn_algo algo)
{
	switch (algo) {
	case NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC:
	case NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA160_HMAC:
		return NSS_IPSECMGR_ALGO_AES_CBC_SHA1_HMAC;

	case NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA160_HMAC:
		return NSS_IPSECMGR_ALGO_3DES_CBC_SHA1_HMAC;

	default:
		nss_cfi_err("Invalid algorithm\n");
		return NSS_IPSECMGR_ALGO_MAX;
	}
}

/*
 * nss_cfi_ipsec_get_natt()
 * 	Get NATT information. Openswan stack fills up NATT flag in skb.
 */
static inline bool nss_cfi_ipsec_get_natt(struct sk_buff *skb)
{
	struct nss_cfi_ipsec_skb_cb *ipsec_cb = (struct nss_cfi_ipsec_skb_cb *)skb->cb;
	return ipsec_cb->natt;
}

/*
 * nss_cfi_ipsec_get_tun_entry()
 * 	get tunnel entry skb. Openswan stack fills up klips dev in skb.
 */
static struct nss_cfi_ipsec_tunnel_entry *nss_cfi_ipsec_get_tun_entry(struct sk_buff *skb)
{
	struct nss_cfi_ipsec_skb_cb *ipsec_cb = (struct nss_cfi_ipsec_skb_cb *)skb->cb;
	int32_t klips_dev_index = ipsec_cb->hlos_dev->ifindex;
	struct nss_cfi_ipsec_tunnel_entry *tun;
	int i;

	/*
	 * Read/write lock needs to taken by the caller since sa
	 * table is looked up here
	 */
	BUG_ON(write_can_lock(&tunnel_map.lock));

	skb->skb_iif = klips_dev_index;

	for (i = 0, tun = tunnel_map.tbl; i < tunnel_map.max; i++, tun++) {
		if (tun->klips_dev_index < 0)
			continue;

		if (klips_dev_index == tun->klips_dev_index)
			return tun;
	}

	return NULL;
}

/*
 * nss_cfi_ipsec_get_index()
 *	given an interface name retrived the numeric suffix
 */
static int16_t nss_cfi_ipsec_get_index(uint8_t *name)
{
	uint8_t *next_char;
	int16_t idx;

	if (strncmp(name, NSS_CFI_IPSEC_BASE_NAME, strlen(NSS_CFI_IPSEC_BASE_NAME)))
		return -1;

	next_char = name + strlen(NSS_CFI_IPSEC_BASE_NAME);
	if ((*next_char < '0') || (*next_char > '9'))
		return -1;

	for (idx = 0; (*next_char >= '0') && (*next_char <= '9'); next_char++)
		idx = (*next_char - '0') + (idx * 10);

	return idx;
}

/*
 * nss_cfi_ipsec_sa_lookup()
 *	Look for an SA based on crypto index.
 */
static struct nss_cfi_ipsec_sa *nss_cfi_ipsec_sa_lookup(struct nss_cfi_ipsec_tunnel_entry *tun,
								uint16_t crypto_idx)
{
	struct list_head *head = &tun->sa_list;
	struct nss_cfi_ipsec_sa *sa;
	struct nss_cfi_ipsec_sa *tmp;

	/*
	 * Read/write lock needs to taken by the caller since sa
	 * table is looked up here
	 */
	BUG_ON(write_can_lock(&tunnel_map.lock));

	list_for_each_entry_safe(sa, tmp, head, list) {
		if (sa->sid == crypto_idx)
			return sa;
	}

	return NULL;
}

/*
 * nss_cfi_ipsec_sa_flush()
 *	Flush all SA entries
 */
static void nss_cfi_ipsec_sa_flush(struct nss_cfi_ipsec_tunnel_entry *tun,
					struct net_device *nss_dev)
{
	struct list_head *head = &tun->sa_list;
	struct nss_cfi_ipsec_sa *sa;
	struct nss_cfi_ipsec_sa *tmp;

	/*
	 * Read/write lock needs to taken by the caller since sa
	 * table is modified here
	 */
	BUG_ON(write_can_lock(&tunnel_map.lock));

	list_for_each_entry_safe(sa, tmp, head, list) {
		list_del_init(&sa->list);
		nss_ipsecmgr_sa_del(nss_dev, &sa->outer);
		kfree(sa);
	}
}

/*
 * nss_cfi_ipsec_free_session()
 *	Free particular session on NSS.
 */
static int32_t nss_cfi_ipsec_free_session(uint32_t crypto_sid)
{
	uint16_t crypto_idx = crypto_sid & NSS_CFI_IPSEC_SES_MASK;
	struct nss_cfi_ipsec_tunnel_entry *tun;
	struct nss_cfi_ipsec_sa *sa = NULL;
	struct net_device *nss_dev;
	int i;

	/*
	 * Write lock needs to be taken here since SA table is
	 * getting modified
	 */
	write_lock_bh(&tunnel_map.lock);

	for (i = 0, tun = tunnel_map.tbl; i < tunnel_map.max; i++, tun++) {
		if ((tun->nss_dev_index < 0) || (tun->klips_dev_index < 0))
			continue;

		sa = nss_cfi_ipsec_sa_lookup(tun, crypto_idx);
		if (sa) {
			list_del_init(&sa->list);
			break;
		}
	}

	if (!sa) {
		write_unlock_bh(&tunnel_map.lock);
		return -ENOENT;
	}

	nss_dev = dev_get_by_index(&init_net, tun->nss_dev_index);
	if (!nss_dev) {
		write_unlock_bh(&tunnel_map.lock);
		kfree(sa);
		return -ENOENT;
	}

	write_unlock_bh(&tunnel_map.lock);

	/*
	 * The SA is now removed from the list hence
	 * we can access it without locks
	 */
	nss_ipsecmgr_sa_del(nss_dev, &sa->outer);
	dev_put(nss_dev);

	kfree(sa);
	return 0;
}

/*
 * nss_cfi_ipsec_fill_outer_flow_v4()
 *	Fill v4 outer flow from skb.
 */
static void *nss_cfi_ipsec_fill_flow_outer(uint8_t *ip_start, uint8_t iv_len, bool natt,
						struct nss_ipsecmgr_flow_outer *outer,
						uint8_t *ttl_hop_limit)
{
	struct iphdr *iph = (struct iphdr *)ip_start;
	struct ipv6hdr *ip6h = (struct ipv6hdr *)ip_start;
	struct ip_esp_hdr *esp;

	/*
	 * KLIPS only sends packets with ESP header for encapsulation.
	 * For decapsulation, the NATT/UDP can still be present.
	 * Note: The NATT/UDP header is added after encrypt.
	 */
	if (iph->version == IPVERSION) {
		uint8_t *esp_start = ip_start + sizeof(*iph);
		esp_start += natt ? sizeof(struct udphdr) : 0;
		esp = (struct ip_esp_hdr *)esp_start;

		outer->src_ip[0] = ntohl(iph->saddr);
		outer->dest_ip[0] = ntohl(iph->daddr);
		outer->spi_index = ntohl(esp->spi);
		outer->sport = natt ? NSS_IPSECMGR_NATT_PORT_DATA : 0;
		outer->dport = natt ? NSS_IPSECMGR_NATT_PORT_DATA : 0;
		outer->proto_next_hdr = natt ? IPPROTO_UDP : IPPROTO_ESP;
		outer->ip_version = IPVERSION;
		*ttl_hop_limit = iph->ttl;

		return (uint8_t *)esp + sizeof(*esp) + iv_len;
	}

	if ((iph->version == 6) && (ip6h->nexthdr == IPPROTO_ESP)) {
		nss_cfi_ipsec_v6addr_ntoh(outer->src_ip, ip6h->saddr.s6_addr32);
		nss_cfi_ipsec_v6addr_ntoh(outer->dest_ip, ip6h->daddr.s6_addr32);

		esp = (struct ip_esp_hdr *)(ip_start + sizeof(*ip6h));
		outer->spi_index = ntohl(esp->spi);
		outer->proto_next_hdr = IPPROTO_ESP;
		outer->ip_version = 6;
		*ttl_hop_limit = ip6h->hop_limit;

		return ip_start + sizeof(*ip6h) + sizeof(*esp) + iv_len;
	}

	return NULL;
}

/*
 * nss_cfi_ipsec_fill_flow_inner()
 *	Fill inner flow
 */
static bool nss_cfi_ipsec_fill_flow_inner(uint8_t *inner_ip, struct nss_ipsecmgr_flow_inner *inner)
{
	struct iphdr *iph = (struct iphdr *)inner_ip;
	struct ipv6hdr *ip6h = (struct ipv6hdr *)inner_ip;
	struct frag_hdr *fragh;

	inner->sport = 0;
	inner->dport = 0;
	inner->use_pattern = 0;

	if (iph->version == IPVERSION) {
		inner->src_ip[0] = ntohl(iph->saddr);
		inner->dest_ip[0] = ntohl(iph->daddr);
		inner->proto_next_hdr = iph->protocol;
		inner->ip_version = IPVERSION;
		goto done;
	}

	if (iph->version != 6)
		return false;

	inner->ip_version = 6;
	nss_cfi_ipsec_v6addr_ntoh(inner->src_ip, ip6h->saddr.s6_addr32);
	nss_cfi_ipsec_v6addr_ntoh(inner->dest_ip, ip6h->daddr.s6_addr32);

	if (ip6h->nexthdr != NEXTHDR_FRAGMENT) {
		inner->proto_next_hdr = ip6h->nexthdr;
		goto done;
	}

	fragh = (struct frag_hdr *)(inner_ip + sizeof(*ip6h));
	inner->proto_next_hdr = fragh->nexthdr;

done:
	return true;
}

/*
 * nss_cfi_ipsec_trap_encap()
 *	Trap IPsec pkts for sending encap fast path rules.
 */
static int32_t nss_cfi_ipsec_trap_encap(struct sk_buff *skb, struct nss_cfi_crypto_info *crypto)
{
	struct nss_ipsecmgr_flow_outer outer = {0};
	struct nss_ipsecmgr_flow_inner inner = {0};
	struct nss_cfi_ipsec_tunnel_entry *tun;
	struct nss_cfi_ipsec_sa *sa_entry;
	struct nss_ipsecmgr_sa sa = {0};
	struct net_device *nss_dev;
	enum nss_ipsecmgr_algo algo;
	uint8_t ttl_hop_limit;
	uint8_t *inner_ip;
	int8_t iv_blk_len;
	uint32_t if_num;

	iv_blk_len = nss_cfi_ipsec_get_iv_blk_len(crypto->algo);
	if (iv_blk_len < 0) {
		nss_cfi_warn("%px:Failed to map valid IV and block length\n", skb);
		return -EOPNOTSUPP;
	}

	algo = nss_cfi_ipsec_get_algo(crypto->algo);
	if (algo >= NSS_IPSECMGR_ALGO_MAX) {
		nss_cfi_warn("%px:Failed to map valid algo\n", skb);
		return -EOPNOTSUPP;
	}

	/*
	 * construct SA information
	 */
	nss_ipsecmgr_sa_cmn_init_idx(&sa.cmn, algo, crypto->sid,
				iv_blk_len, iv_blk_len, crypto->hash_len,
				false,				/* secure_key */
				false,				/* no_trailer */
				false,				/* esn */
				nss_cfi_ipsec_get_natt(skb)	/* natt */
				);

	/*
	 * KLIPS adds NATT/UDP header after encrypt.
	 */
	inner_ip = nss_cfi_ipsec_fill_flow_outer(skb->data, iv_blk_len, false, &outer, &ttl_hop_limit);
	if (!inner_ip) {
		nss_cfi_warn("%px:Failed to fill outer flow rule\n", skb);
		return -EINVAL;
	}

	if (!nss_cfi_ipsec_fill_flow_inner(inner_ip, &inner)) {
		nss_cfi_warn("%px:Failed to fill inner flow rule\n", skb);
		return -EINVAL;
	}

	sa.type = NSS_IPSECMGR_SA_TYPE_ENCAP;
	sa.encap.ttl_hop_limit = ttl_hop_limit;

	/*
	 * Read lock needs to be taken here as the tunnel map table
	 * is looked up
	 */
	write_lock(&tunnel_map.lock);

	tun = nss_cfi_ipsec_get_tun_entry(skb);
	if (!tun) {
		write_unlock(&tunnel_map.lock);
		nss_cfi_warn("%px:Failed to find NSS device mapped to KLIPS device\n", skb);
		return -ENOENT;
	}

	nss_dev = dev_get_by_index(&init_net, tun->nss_dev_index);
	if (!nss_dev) {
		write_unlock(&tunnel_map.lock);
		nss_cfi_warn("%px:Failed to find NSS device(%d) in Linux\n",
						skb, tun->nss_dev_index);
		return -ENOENT;
	}

	sa_entry = nss_cfi_ipsec_sa_lookup(tun, crypto->sid);
	if (sa_entry)
		goto flow_add;

	sa_entry = kzalloc(sizeof(*sa_entry), GFP_ATOMIC);
	if (!sa_entry) {
		write_unlock(&tunnel_map.lock);
		dev_put(nss_dev);
		return -ENOMEM;
	}

	/*
	 * If, SA add fails then there is no need to add the flow
	 */
	if (nss_ipsecmgr_sa_add(nss_dev, &outer, &sa, &if_num) != NSS_IPSECMGR_OK) {
		write_unlock(&tunnel_map.lock);
		goto sa_free;
	}

	sa_entry->sid = crypto->sid;
	memcpy(&sa_entry->outer, &outer, sizeof(sa_entry->outer));

	INIT_LIST_HEAD(&sa_entry->list);
	list_add_tail(&sa_entry->list, &tun->sa_list);

flow_add:

	/*
	 * If flow add fails due to lack of an IPsec manager SA entry, we need to remove
	 * IPsec SA entry too and unwind any previous SA programming
	 */
	if (nss_ipsecmgr_flow_add(nss_dev, &inner, &outer) == NSS_IPSECMGR_FAIL_SA) {
		list_del_init(&sa_entry->list);
		write_unlock(&tunnel_map.lock);
		goto sa_free;
	}

	dev_put(nss_dev);
	write_unlock(&tunnel_map.lock);

	nss_cfi_dbg("Encap SA rule pushed successfully\n");
	return 0;

sa_free:
	kfree(sa_entry);
	dev_put(nss_dev);
	return -EINVAL;
}

/*
 * nss_cfi_ipsec_trap_decap()
 *	Trap IPsec pkts for sending decap fast path rules.
 */
static int32_t nss_cfi_ipsec_trap_decap(struct sk_buff *skb, struct nss_cfi_crypto_info *crypto)
{
	struct nss_ipsecmgr_flow_outer outer = {0};
	struct nss_ipsecmgr_flow_inner inner = {0};
	struct nss_cfi_ipsec_tunnel_entry *tun;
	struct nss_cfi_ipsec_sa *sa_entry;
	struct nss_ipsecmgr_sa sa = {0};
	struct net_device *nss_dev;
	enum nss_ipsecmgr_algo algo;
	uint8_t ttl_hop_limit;
	uint8_t *inner_ip;
	int8_t iv_blk_len;
	uint32_t if_num;
	bool natt;

	iv_blk_len = nss_cfi_ipsec_get_iv_blk_len(crypto->algo);
	if (iv_blk_len < 0) {
		nss_cfi_warn("%px:Failed to map valid IV and block length\n", skb);
		return -EOPNOTSUPP;
	}

	algo = nss_cfi_ipsec_get_algo(crypto->algo);
	if (algo >= NSS_IPSECMGR_ALGO_MAX) {
		nss_cfi_warn("%px:Failed to map valid algo\n", skb);
		return -EOPNOTSUPP;
	}

	/*
	 * construct SA information
	 */
	natt = nss_cfi_ipsec_get_natt(skb);
	nss_ipsecmgr_sa_cmn_init_idx(&sa.cmn, algo, crypto->sid,
				iv_blk_len, iv_blk_len, crypto->hash_len,
				false,	/* secure_key */
				false,	/* no_trailer */
				false,	/* esn */
				natt	/* natt */
				);

	/*
	 * construct outer flow information
	 */
	inner_ip = nss_cfi_ipsec_fill_flow_outer(skb_network_header(skb), iv_blk_len, natt, &outer, &ttl_hop_limit);
	if (!inner_ip) {
		nss_cfi_warn("%px:Failed to fill outer flow rule\n", skb);
		return -EINVAL;
	}

	if (!nss_cfi_ipsec_fill_flow_inner(inner_ip, &inner)) {
		nss_cfi_warn("%px:Failed to fill inner flow rule\n", skb);
		return -EINVAL;
	}

	sa.type = NSS_IPSECMGR_SA_TYPE_DECAP;

	/*
	 * Read lock needs to be taken here as the tunnel map table
	 * is looked up
	 */
	write_lock(&tunnel_map.lock);

	tun = nss_cfi_ipsec_get_tun_entry(skb);
	if (!tun) {
		write_unlock(&tunnel_map.lock);
		nss_cfi_warn("%px:Failed to find NSS device mapped to KLIPS device\n", skb);
		return -ENOENT;
	}

	nss_dev = dev_get_by_index(&init_net, tun->nss_dev_index);
	if (!nss_dev) {
		write_unlock(&tunnel_map.lock);
		nss_cfi_warn("%px:Failed to find NSS device(%d) in Linux\n",
							skb, tun->nss_dev_index);
		return -ENOENT;
	}

	sa_entry = nss_cfi_ipsec_sa_lookup(tun, crypto->sid);
	if (sa_entry)
		goto flow_add;

	sa_entry = kzalloc(sizeof(*sa_entry), GFP_ATOMIC);
	if (!sa_entry) {
		write_unlock(&tunnel_map.lock);
		dev_put(nss_dev);
		return -ENOMEM;
	}

	/*
	 * If, SA add fails then there is no need to add the flow
	 */
	if (nss_ipsecmgr_sa_add(nss_dev, &outer, &sa, &if_num) != NSS_IPSECMGR_OK) {
		write_unlock(&tunnel_map.lock);
		goto sa_free;
	}

	sa_entry->sid = crypto->sid;
	memcpy(&sa_entry->outer, &outer, sizeof(sa_entry->outer));

	INIT_LIST_HEAD(&sa_entry->list);
	list_add_tail(&sa_entry->list, &tun->sa_list);

flow_add:

	/*
	 * If flow add fails due to lack of an IPsec manager SA entry,
	 * we need to remove IPsec SA entry too
	 */
	if (nss_ipsecmgr_flow_add(nss_dev, &inner, &outer) == NSS_IPSECMGR_FAIL_SA) {
		list_del_init(&sa_entry->list);
		write_unlock(&tunnel_map.lock);
		goto sa_free;
	}

	dev_put(nss_dev);
	write_unlock(&tunnel_map.lock);

	nss_cfi_dbg("Decap SA rule pushed successfully\n");
	return 0;

sa_free:
	kfree(sa_entry);
	dev_put(nss_dev);
	return -EINVAL;
}

/*
 * nss_cfi_ipsec_offload()
 * 	Offload function for encapsulation.
 */
static nss_cfi_offload_status_t nss_cfi_ipsec_offload(struct sk_buff *skb, struct nss_cfi_crypto_info *crypto)
{
	/*
	 * We don't support offloading.
	 */
	return -1;
}

/*
 * nss_cfi_ipsec_dev_event()
 *	notifier function for IPsec device events.
 */
static int nss_cfi_ipsec_dev_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct net_device *klips_dev = netdev_notifier_info_to_dev(ptr);
	struct nss_ipsecmgr_callback ipsec_cb = {0};
	struct nss_cfi_ipsec_tunnel_entry *tun;
	struct net_device *nss_dev;
	int16_t index = 0;

	if (tunnel_map.max <= tunnel_map.used)
		return NOTIFY_DONE;

	switch (event) {
	case NETDEV_REGISTER:
		index = nss_cfi_ipsec_get_index(klips_dev->name);
		if ((index < 0) || (index >= tunnel_map.max)) {
			nss_cfi_trace("Netdev(%s) is not KLIPS IPsec related\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		/*
		 * All exception packets are handled by IPsec manager and
		 * hence, the lack of need to register a data callback.
		 * The event callback can be used, if the need arises
		 * to send an asynchronous event to OCF layer.
		 */
		ipsec_cb.skb_dev = klips_dev;
		ipsec_cb.data_cb = NULL;
		ipsec_cb.event_cb = NULL;

		nss_cfi_info("IPsec interface being registered: %s\n", klips_dev->name);

		nss_dev = nss_ipsecmgr_tunnel_add(&ipsec_cb);
		if (!nss_dev) {
			nss_cfi_err("NSS IPsec tunnel dev allocation failed for %s\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		/*
		 * Write lock is needed here since tunnel map table
		 * is modified
		 */
		write_lock_bh(&tunnel_map.lock);

		tunnel_map.used++;
		tunnel_map.tbl[index].nss_dev_index = nss_dev->ifindex;
		tunnel_map.tbl[index].klips_dev_index = klips_dev->ifindex;

		write_unlock_bh(&tunnel_map.lock);
		break;

	case NETDEV_UNREGISTER:
		index = nss_cfi_ipsec_get_index(klips_dev->name);
		if ((index < 0) || (index >= tunnel_map.max)) {
			nss_cfi_trace("Netdev(%s) is not KLIPS IPsec related\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		/*
		 * Read lock necessary as tunnel map table is accessed but
		 * not modified
		 */
		write_lock_bh(&tunnel_map.lock);
		tun = &tunnel_map.tbl[index];

		if ((tun->klips_dev_index < 0) || (tun->nss_dev_index < 0)) {
			write_unlock_bh(&tunnel_map.lock);
			nss_cfi_err("%px:Failed to find tunnel map\n", klips_dev);
			return NOTIFY_DONE;
		}

		if (tun->klips_dev_index != klips_dev->ifindex) {
			write_unlock_bh(&tunnel_map.lock);
			nss_cfi_err("Failed to find NSS IPsec tunnel dev for %s\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		/*
		 * Since, we created the NSS device then we should not never see this happen
		 */
		nss_dev = dev_get_by_index(&init_net, tun->nss_dev_index);
		BUG_ON(!nss_dev);

		nss_cfi_info("IPsec interface being unregistered: %s\n", klips_dev->name);

		nss_cfi_ipsec_sa_flush(tun, nss_dev);

		/*
		 * Write lock is needed here since tunnel map table
		 * is modified
		 */
		tun->nss_dev_index = -1;
		tun->klips_dev_index = -1;
		tunnel_map.used--;

		write_unlock_bh(&tunnel_map.lock);

		nss_ipsecmgr_tunnel_del(nss_dev);
		dev_put(nss_dev);
		break;

	case NETDEV_CHANGEMTU:
		index = nss_cfi_ipsec_get_index(klips_dev->name);

		if ((index < 0) || (index >= tunnel_map.max)) {
			nss_cfi_trace("Netdev(%s) is not KLIPS IPsec related\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		/*
		 * Read lock necessary as tunnel map table is accessed but
		 * not modified
		 */
		write_lock_bh(&tunnel_map.lock);
		tun = &tunnel_map.tbl[index];

		if ((tun->klips_dev_index < 0) || (tun->nss_dev_index < 0)) {
			write_unlock_bh(&tunnel_map.lock);
			nss_cfi_err("%px:Failed to find tunnel map\n", klips_dev);
			return NOTIFY_DONE;
		}

		if (tun->klips_dev_index != klips_dev->ifindex) {
			write_unlock_bh(&tunnel_map.lock);
			nss_cfi_err("Failed to find NSS IPsec tunnel dev for %s\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		nss_dev = dev_get_by_index(&init_net, tun->nss_dev_index);
		BUG_ON(!nss_dev);
		write_unlock_bh(&tunnel_map.lock);

		dev_set_mtu(nss_dev, klips_dev->mtu);
		dev_put(nss_dev);
		break;

	default:
		return NOTIFY_DONE;
	}

	return NOTIFY_OK;
}

static struct notifier_block nss_cfi_ipsec_notifier = {
	.notifier_call = nss_cfi_ipsec_dev_event,
};

/*
 * nss_cfi_ipsec_init_module()
 *	Initialize IPsec rule tables and register various callbacks
 */
int __init nss_cfi_ipsec_init_module(void)
{
	struct nss_cfi_ipsec_tunnel_entry *tun;
	int i;

	nss_cfi_info("NSS IPsec (platform - IPQ807x , %s) loaded\n", NSS_CFI_BUILD_ID);

	tunnel_map.tbl = vzalloc(sizeof(struct nss_cfi_ipsec_tunnel_entry) * tunnel_max);
	if (!tunnel_map.tbl) {
		nss_cfi_warn("Unable to allocate tunnel map table\n");
		return -1;
	}

	for (i = 0, tun = tunnel_map.tbl; i < tunnel_max; i++, tun++) {
		tun->nss_dev_index = -1;
		tun->klips_dev_index = -1;
		INIT_LIST_HEAD(&tun->sa_list);
	}

	rwlock_init(&tunnel_map.lock);

	tunnel_map.max = tunnel_max;
	tunnel_map.used = 0;

	register_netdevice_notifier(&nss_cfi_ipsec_notifier);
	nss_cfi_ocf_register_ipsec(nss_cfi_ipsec_trap_encap, nss_cfi_ipsec_trap_decap, nss_cfi_ipsec_free_session,
				nss_cfi_ipsec_offload);

	return 0;
}

/*
 * nss_cfi_ipsec_exit_module()
 *	Unregister callbacks/notifiers and clear all stale data
 */
void __exit nss_cfi_ipsec_exit_module(void)
{
	struct nss_cfi_ipsec_tunnel_entry *tun;
	struct net_device *nss_dev;
	int i;

	/*
	 * Detach the trap handlers and Linux NETDEV notifiers, before
	 * unwinding the tunnels
	 */
	nss_cfi_ocf_unregister_ipsec();
	unregister_netdevice_notifier(&nss_cfi_ipsec_notifier);

	/*
	 * Write lock needs to be taken here since SA table is
	 * getting modified
	 */
	write_lock_bh(&tunnel_map.lock);

	for (i = 0, tun = tunnel_map.tbl; i < tunnel_map.max; i++, tun++) {
		if ((tun->nss_dev_index < 0) || (tun->klips_dev_index < 0))
			continue;

		/*
		 * Find the NSS device associated with klips device
		 */
		nss_dev = dev_get_by_index(&init_net, tun->nss_dev_index);
		if (nss_dev) {
			nss_cfi_ipsec_sa_flush(tun, nss_dev);
			nss_ipsecmgr_tunnel_del(nss_dev);
			dev_put(nss_dev);
		}

		BUG_ON(!list_empty(&tun->sa_list));

		/*
		 * Reset the tunnel entry
		 */
		tun->nss_dev_index = -1;
		tun->klips_dev_index = -1;
		tunnel_map.used--;
	}

	write_unlock_bh(&tunnel_map.lock);

	tunnel_map.used = tunnel_max;
	tunnel_map.max = 0;

	vfree(tunnel_map.tbl);
	nss_cfi_info("module unloaded\n");
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS IPsec offload glue");

module_init(nss_cfi_ipsec_init_module);
module_exit(nss_cfi_ipsec_exit_module);
