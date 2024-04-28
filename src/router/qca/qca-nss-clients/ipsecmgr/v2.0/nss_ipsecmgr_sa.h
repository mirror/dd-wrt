/*
 * ********************************************************************************
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
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
 **********************************************************************************
 */

#ifndef __NSS_IPSECMGR_SA_H
#define __NSS_IPSECMGR_SA_H

#define NSS_IPSECMGR_SA_MAX 64			/**< Maximum number of SA(s) */
#if (~(NSS_IPSECMGR_SA_MAX - 1) & (NSS_IPSECMGR_SA_MAX >> 1))
#error "NSS_IPSECMGR_SA_MAX is not a power of 2"
#endif

#define NSS_IPSECMGR_SA_PRINT_EXTRA 128
#define NSS_IPSECMGR_SA_FREE_TIMEOUT msecs_to_jiffies(40) /* msecs */

#define NSS_IPSECMGR_ESP_TRAIL_SZ 2 /* esp trailer size */
#define NSS_IPSECMGR_ESP_PAD_SZ 14 /* maximum amount of padding */

/*
 * IPsec manager packets stats per SA
 */
struct nss_ipsecmgr_sa_stats_priv {
	/* Packet counters */
	uint64_t rx_packets;			/**< Number of packets received. */
	uint64_t rx_bytes;			/**< Number of bytes received. */
	uint64_t tx_packets;			/**< Number of packets transmitted. */
	uint64_t tx_bytes;			/**< Number of bytes transmitted. */
	uint64_t rx_dropped[NSS_MAX_NUM_PRI];	/**< Packets dropped on receive due to queue full. */

	/* Drop counters */
	uint64_t fail_headroom;			/**< Failed headroom check. */
	uint64_t fail_tailroom;			/**< Failed tailroom check. */
	uint64_t fail_replay;			/**< Failure in anti-replay check. */
	uint64_t fail_replay_dup;		/**< Failure in anti-replay; duplicate records. */
	uint64_t fail_replay_win;		/**< Failure in anti-replay; packet outside the window. */
	uint64_t fail_pbuf_crypto;		/**< Failed to allocate crypto pbuf. */
	uint64_t fail_queue;			/**< Failure due to queue full in IPsec. */
	uint64_t fail_queue_crypto;		/**< Failure due to queue full in crypto. */
	uint64_t fail_queue_nexthop;		/**< Failure due to queue full in next_hop. */
	uint64_t fail_pbuf_alloc;		/**< Failure in pbuf allocation. */
	uint64_t fail_pbuf_linear;		/**< Failure in pbuf linearization. */
	uint64_t fail_pbuf_stats;		/**< Failure in pbuf allocation for stats. */
	uint64_t fail_pbuf_align;		/**< Failure in pbuf access due non-word alignment. */
	uint64_t fail_cipher;			/**< Failure in decrypting the data. */
	uint64_t fail_auth;			/**< Failure in authenticating the data. */
	uint64_t fail_seq_ovf;			/**< Failure due sequence no. roll over. */
	uint64_t fail_blk_len;			/**< Failure in decap due to bad cipher block len. */
	uint64_t fail_hash_len;			/**< Failure in decap due to bad hash block len. */
	uint64_t fail_transform;		/**< Failure in transformation; general error. */
	uint64_t fail_crypto;			/**< Failure in crypto transformation. */
};

/*
 * IPsec manager SA state
 */
struct nss_ipsecmgr_sa_state {
	struct nss_ipsec_cmn_sa_tuple tuple;		/* SA tuple */
	struct nss_ipsec_cmn_sa_data data;		/* SA data */
	struct nss_ipsec_cmn_sa_replay replay;		/* Per SA replay data */
	bool tx_default;				/* SA used for tunnel TX */
};

/*
 * IPsec manager SA entry
 */
struct nss_ipsecmgr_sa {
	struct list_head list;				/* List node */
	struct nss_ipsecmgr_ref ref;			/* Reference node */

	struct crypto_aead *aead;			/* Linux crypto AEAD context */
	struct crypto_ahash *ahash;			/* Linux crypto AHASH context */

	uint32_t ifnum;					/* Interface number */
	struct delayed_work free_work;			/* Delayed free work */
	enum nss_ipsec_cmn_ctx_type type;		/* Type */
	struct nss_ctx_instance *nss_ctx;		/* NSS context */

	struct nss_ipsecmgr_sa_state state;		/* SA local state */
	struct nss_ipsecmgr_sa_stats_priv stats;	/* SA statistics */

	int tunnel_id;					/* Linux device index */
	struct nss_ipsecmgr_callback cb;		/* Callback entry */
};

/*
 * nss_ipsecmgr_sa_ntoh_v6addr()
 *	Network to host order
 */
static inline void nss_ipsecmgr_sa_ntoh_v6addr(uint32_t *dest, uint32_t *src)
{
	dest[0] = ntohl(src[0]);
	dest[1] = ntohl(src[1]);
	dest[2] = ntohl(src[2]);
	dest[3] = ntohl(src[3]);
}

/*
 * nss_ipsecmgr_sa_tuple2index()
 * 	Change tuple to hash index
 */
static inline uint32_t nss_ipsecmgr_sa_tuple2hash(struct nss_ipsec_cmn_sa_tuple *tuple, uint32_t max)
{
	uint32_t val = 0;

	val ^= tuple->dest_ip[0];
	val ^= tuple->src_ip[0];
	val ^= tuple->dest_ip[1];
	val ^= tuple->src_ip[1];
	val ^= tuple->dest_ip[2];
	val ^= tuple->src_ip[2];
	val ^= tuple->dest_ip[3];
	val ^= tuple->src_ip[3];
	val ^= tuple->spi_index;
	val ^= tuple->protocol;
	val ^= tuple->ip_ver;

	return val & (max - 1);
}

/*
 * nss_ipsecmgr_sa_tuple_match()
 * 	Match flow tuple
 */
static inline bool nss_ipsecmgr_sa_tuple_match(struct nss_ipsec_cmn_sa_tuple *d, struct nss_ipsec_cmn_sa_tuple *s)
{
	uint32_t status = 0;

	switch (d->ip_ver) {
	case IPVERSION:
		status += d->dest_ip[0] ^ s->dest_ip[0];
		status += d->src_ip[0] ^ s->src_ip[0];
		status += d->spi_index ^ s->spi_index;
		status += d->protocol ^ s->protocol;
		status += d->ip_ver ^ s->ip_ver;
		return !status;

	default:
		status += d->dest_ip[0] ^ s->dest_ip[0];
		status += d->dest_ip[1] ^ s->dest_ip[1];
		status += d->dest_ip[2] ^ s->dest_ip[2];
		status += d->dest_ip[3] ^ s->dest_ip[3];
		status += d->src_ip[0] ^ s->src_ip[0];
		status += d->src_ip[1] ^ s->src_ip[1];
		status += d->src_ip[2] ^ s->src_ip[2];
		status += d->src_ip[3] ^ s->src_ip[3];
		status += d->spi_index ^ s->spi_index;
		status += d->protocol ^ s->protocol;
		status += d->ip_ver ^ s->ip_ver;
		return !status;
	}
}

/*
 * nss_ipsecmgr_sa_tuple2sa()
 *	Convert tuple to sa
 */
static inline void nss_ipsecmgr_sa_tuple2sa(struct nss_ipsec_cmn_sa_tuple *tuple, struct nss_ipsecmgr_sa_tuple *sa)
{
	memcpy(sa->dest_ip, tuple->dest_ip, sizeof(sa->dest_ip));
	memcpy(sa->src_ip, tuple->src_ip, sizeof(sa->src_ip));
	sa->spi_index = tuple->spi_index;
	sa->proto_next_hdr = tuple->protocol;
	sa->ip_version = tuple->ip_ver;
	sa->sport = tuple->src_port;
	sa->dport = tuple->dest_port;
}

/*
 * nss_ipsecmgr_sa2tuple()
 *	Convert sa to tuple
 */
static inline void nss_ipsecmgr_sa2tuple(struct nss_ipsecmgr_sa_tuple *sa, struct nss_ipsec_cmn_sa_tuple *tuple)
{
	memcpy(tuple->dest_ip, sa->dest_ip, sizeof(tuple->dest_ip));
	memcpy(tuple->src_ip, sa->src_ip, sizeof(tuple->src_ip));
	tuple->spi_index = sa->spi_index;
	tuple->protocol = sa->proto_next_hdr;
	tuple->ip_ver = sa->ip_version;
	tuple->src_port = sa->sport;
	tuple->dest_port = sa->dport;
}

/*
 * nss_ipsecmgr_sa_ipv4_outer2tuple()
 * 	Change outer IPv4 SA to tuple
 */
static inline void nss_ipsecmgr_sa_ipv4_outer2tuple(struct iphdr *iph, struct nss_ipsec_cmn_sa_tuple *s)
{
	uint8_t *data = (uint8_t *)iph;
	struct ip_esp_hdr *esph;

	WARN_ON((iph->protocol != IPPROTO_ESP) && (iph->protocol != IPPROTO_UDP));

	s->src_ip[0] = ntohl(iph->saddr);
	s->dest_ip[0] = ntohl(iph->daddr);
	s->ip_ver = IPVERSION;
	s->protocol = iph->protocol;

	data += sizeof(*iph);

	if (s->protocol == IPPROTO_UDP)
		data += sizeof(struct udphdr);

	esph = (struct ip_esp_hdr *)data;
	s->spi_index = ntohl(esph->spi);
}

/*
 * nss_ipsecmgr_sa_ipv6_outer2tuple()
 * 	Change inner IPv6 SA to tuple
 */
static inline void nss_ipsecmgr_sa_ipv6_outer2tuple(struct ipv6hdr *ip6h, struct nss_ipsec_cmn_sa_tuple *s)
{
	uint8_t *data = (uint8_t *)ip6h;
	struct ip_esp_hdr *esph;

	nss_ipsecmgr_sa_ntoh_v6addr(s->src_ip, ip6h->saddr.s6_addr32);
	nss_ipsecmgr_sa_ntoh_v6addr(s->dest_ip, ip6h->daddr.s6_addr32);

	s->protocol = ip6h->nexthdr;
	s->ip_ver = 6;

	WARN_ON(ip6h->nexthdr != IPPROTO_ESP);

	data += sizeof(*ip6h);

	esph = (struct ip_esp_hdr *)data;
	s->spi_index = ntohl(esph->spi);
}

/* functions to operate on SA object */
struct nss_ipsecmgr_sa *nss_ipsecmgr_sa_find(struct list_head *db, struct nss_ipsec_cmn_sa_tuple *tuple);
void nss_ipsecmgr_sa_sync_state(struct nss_ipsecmgr_sa *sa, struct nss_ipsec_cmn_sa_sync *sync);
void nss_ipsecmgr_sa_sync2stats(struct nss_ipsec_cmn_sa_sync *sync, struct nss_ipsecmgr_sa_stats *stats);
#endif
