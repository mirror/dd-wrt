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
#ifndef __NSS_IPSECMGR_FLOW_H
#define __NSS_IPSECMGR_FLOW_H

#define NSS_IPSECMGR_FLOW_MAX 1024 /* Maximum number of flow(s) */
#if (~(NSS_IPSECMGR_FLOW_MAX - 1) & (NSS_IPSECMGR_FLOW_MAX >> 1))
#error "NSS_IPSECMGR_FLOW_MAX is not a power of 2"
#endif

#define NSS_IPSECMGR_FLOW_PRINT_EXTRA 64
#define NSS_IPSECMGR_FLOW_FREE_TIMEOUT msecs_to_jiffies(20) /* msecs */

struct nss_ipsecmgr_sa;

/*
 * Per flow state
 */
struct nss_ipsecmgr_flow_state {
	struct nss_ipsec_cmn_flow_tuple tuple;	/* Flow tuple */
	struct nss_ipsec_cmn_sa_tuple sa;	/* SA tuple; used during deletion */
};

/*
 * IPsec manager flow entry
 */
struct nss_ipsecmgr_flow {
	struct list_head list;			/* List object. */
	struct nss_ipsecmgr_ref ref;		/* Reference object. */
	struct nss_ipsecmgr_sa *sa;		/* Parent SA object */

	uint32_t ifnum;				/* NSS interface attached to flow */
	struct delayed_work free_work;		/* Retry work */
	struct nss_ctx_instance *nss_ctx;	/* NSS context */
	struct nss_ipsecmgr_flow_state state;

	int tunnel_id;				/* Linux device index */
};

/*
 * nss_ipsecmgr_flow_ntoh_v6addr()
 *	Network to host order and swap
 */
static inline void nss_ipsecmgr_flow_ntoh_v6addr(uint32_t *dest, uint32_t *src)
{
	dest[0] = ntohl(src[0]);
	dest[1] = ntohl(src[1]);
	dest[2] = ntohl(src[2]);
	dest[3] = ntohl(src[3]);
}

/*
 * nss_ipsecmgr_flow_tuple_match()
 * 	Match flow tuple
 */
static inline bool nss_ipsecmgr_flow_tuple_match(struct nss_ipsec_cmn_flow_tuple *d, struct nss_ipsec_cmn_flow_tuple *s)
{
	uint32_t status = 0;

	switch (d->ip_ver) {
	case IPVERSION:
		status += d->dest_ip[0] ^ s->dest_ip[0];
		status += d->src_ip[0] ^ s->src_ip[0];
		status += d->spi_index ^ s->spi_index;
		status += d->protocol ^ s->protocol;
		status += d->ip_ver ^ s->ip_ver;
		status += d->user_pattern ^ s->user_pattern;
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
		status += d->user_pattern ^ s->user_pattern;
		return !status;
	}
}

/*
 * nss_ipsecmgr_flow_tuple2index()
 * 	Change tuple to hash index
 */
static inline uint32_t nss_ipsecmgr_flow_tuple2hash(struct nss_ipsec_cmn_flow_tuple *tuple, uint32_t max)
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
 * nss_ipsecmgr_flow2tuple()
 * 	Change flow to tuple
 */
static inline void nss_ipsecmgr_flow2tuple(struct nss_ipsecmgr_flow_tuple *f, struct nss_ipsec_cmn_flow_tuple *t)
{
	memcpy(t->dest_ip, f->dest_ip, sizeof(t->dest_ip));
	memcpy(t->src_ip, f->src_ip, sizeof(t->src_ip));
	t->spi_index = f->spi_index;
	t->protocol = f->proto_next_hdr;
	t->ip_ver = f->ip_version;
	t->src_port = f->sport;
	t->dst_port = f->dport;
	t->user_pattern = f->use_pattern;
}

/*
 * nss_ipsecmgr_flow_ipv4_inner2tuple()
 * 	Change inner IPv4 flow to tuple
 */
static inline void nss_ipsecmgr_flow_ipv4_inner2tuple(struct iphdr *iph, struct nss_ipsec_cmn_flow_tuple *t)
{
	t->src_ip[0] = ntohl(iph->saddr);
	t->dest_ip[0] = ntohl(iph->daddr);
	t->ip_ver = IPVERSION;
	t->protocol = iph->protocol;
}

/*
 * nss_ipsecmgr_flow_ipv4_outer2tuple()
 * 	Change outer IPv4 flow to tuple
 */
static inline void nss_ipsecmgr_flow_ipv4_outer2tuple(struct iphdr *iph, struct nss_ipsec_cmn_flow_tuple *t)
{
	uint8_t *data = (uint8_t *)iph;
	struct ip_esp_hdr *esph;

	WARN_ON((iph->protocol != IPPROTO_ESP) && (iph->protocol != IPPROTO_UDP));

	t->src_ip[0] = ntohl(iph->saddr);
	t->dest_ip[0] = ntohl(iph->daddr);
	t->ip_ver = IPVERSION;
	t->protocol = iph->protocol;

	data += sizeof(*iph);

	if (t->protocol == IPPROTO_UDP)
		data += sizeof(struct udphdr);

	esph = (struct ip_esp_hdr *)data;
	t->spi_index = ntohl(esph->spi);
}

/*
 * nss_ipsecmgr_flow_ipv6_inner2tuple()
 * 	Change inner IPv6 flow to tuple
 */
static inline void nss_ipsecmgr_flow_ipv6_inner2tuple(struct ipv6hdr *ip6h, struct nss_ipsec_cmn_flow_tuple *t)
{
	uint8_t *data = (uint8_t *)ip6h;
	struct frag_hdr *fragh;

	nss_ipsecmgr_flow_ntoh_v6addr(t->src_ip, ip6h->saddr.s6_addr32);
	nss_ipsecmgr_flow_ntoh_v6addr(t->dest_ip, ip6h->daddr.s6_addr32);

	t->protocol = ip6h->nexthdr;
	t->ip_ver = 6;

	data += sizeof(*ip6h);

	if (t->protocol == NEXTHDR_FRAGMENT) {
		fragh = (struct frag_hdr *)data;
		t->protocol = fragh->nexthdr;
	}
}

/*
 * nss_ipsecmgr_flow_ipv6_outer2tuple()
 * 	Change inner IPv6 flow to tuple
 */
static inline void nss_ipsecmgr_flow_ipv6_outer2tuple(struct ipv6hdr *ip6h, struct nss_ipsec_cmn_flow_tuple *t)
{
	uint8_t *data = (uint8_t *)ip6h;
	struct ip_esp_hdr *esph;

	nss_ipsecmgr_flow_ntoh_v6addr(t->src_ip, ip6h->saddr.s6_addr32);
	nss_ipsecmgr_flow_ntoh_v6addr(t->dest_ip, ip6h->daddr.s6_addr32);

	t->protocol = ip6h->nexthdr;
	t->ip_ver = 6;

	WARN_ON(ip6h->nexthdr != IPPROTO_ESP);

	data += sizeof(*ip6h);

	esph = (struct ip_esp_hdr *)data;
	t->spi_index = ntohl(esph->spi);
}

struct nss_ipsecmgr_flow *nss_ipsecmgr_flow_find(struct list_head *db, struct nss_ipsec_cmn_flow_tuple *t);
#endif
