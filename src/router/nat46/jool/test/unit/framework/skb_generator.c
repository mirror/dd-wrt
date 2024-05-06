#include "framework/skb_generator.h"
#include "framework/types.h"
#include "mod/common/log.h"

#include <linux/if_ether.h>
#include <linux/ipv6.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/icmp.h>


static int store_bits(struct sk_buff *skb, int *offset, void *bits, size_t len)
{
	int room;
	int delta;
	int error;

	if ((*offset) >= skb->len)
		return 0; /* Caller wants the packet truncated. */

	room = skb->len - (*offset);
	delta = (room > len) ? len : room;
	error = skb_store_bits(skb, *offset, bits, delta);
	if (error) {
		log_err("skb_store_bits() error: %d", error);
		return error;
	}

	*offset += delta;
	return 0;
}

#define HDR4_LEN sizeof(struct iphdr)
int init_ipv4_hdr(struct sk_buff *skb, int *offset, char *src, char *dst,
		u16 payload_len, u8 nexthdr, u8 ttl)
{
	struct iphdr hdr;
	struct in_addr tmp;
	int error;

	hdr.version = 4;
	hdr.ihl = 5;
	hdr.tos = 0;
	hdr.tot_len = cpu_to_be16(sizeof(hdr) + payload_len);
	hdr.id = 0;
	hdr.frag_off = build_ipv4_frag_off_field(1, 0, 0);
	hdr.ttl = ttl;
	hdr.protocol = nexthdr;

	error = str_to_addr4(src, &tmp);
	if (error)
		return error;
	hdr.saddr = tmp.s_addr;

	error = str_to_addr4(dst, &tmp);
	if (error)
		return error;
	hdr.daddr = tmp.s_addr;

	hdr.check = 0;
	hdr.check = ip_fast_csum(&hdr, hdr.ihl);

	return store_bits(skb, offset, &hdr, sizeof(hdr));
}

#define HDR6_LEN sizeof(struct ipv6hdr)
int init_ipv6_hdr(struct sk_buff *skb, int *offset, char *src, char *dst,
		u16 payload_len, u8 nexthdr, u8 ttl)
{
	struct ipv6hdr hdr;
	int error;

	hdr.version = 6;
	hdr.priority = 0;
	hdr.flow_lbl[0] = 0;
	hdr.flow_lbl[1] = 0;
	hdr.flow_lbl[2] = 0;
	hdr.payload_len = cpu_to_be16(payload_len);
	hdr.nexthdr = nexthdr;
	hdr.hop_limit = ttl;
	error = str_to_addr6(src, &hdr.saddr);
	if (error)
		return error;
	error = str_to_addr6(dst, &hdr.daddr);
	if (error)
		return error;

	return store_bits(skb, offset, &hdr, sizeof(hdr));
}

#define UDP_HDR_LEN sizeof(struct udphdr)
static int init_udp_hdr(struct sk_buff *skb, int *offset, __u16 src, __u16 dst,
		u16 datagram_len)
{
	struct udphdr hdr;

	hdr.source = cpu_to_be16(src);
	hdr.dest = cpu_to_be16(dst);
	hdr.len = cpu_to_be16(datagram_len);
	hdr.check = 0;

	return store_bits(skb, offset, &hdr, sizeof(hdr));
}

#define TCP_HDR_LEN sizeof(struct tcphdr)
int init_tcp_hdr(struct sk_buff *skb, int *offset, __u16 src, __u16 dst,
		u16 datagram_len)
{
	struct tcphdr hdr;

	hdr.source = cpu_to_be16(src);
	hdr.dest = cpu_to_be16(dst);
	hdr.seq = cpu_to_be32(4669);
	hdr.ack_seq = cpu_to_be32(6576);
	hdr.doff = sizeof(hdr) / 4;
	hdr.res1 = 0;
	hdr.cwr = 0;
	hdr.ece = 0;
	hdr.urg = 0;
	hdr.ack = 0;
	hdr.psh = 0;
	hdr.rst = 0;
	hdr.syn = 1;
	hdr.fin = 0;
	hdr.window = cpu_to_be16(3233);
	hdr.check = 0;
	hdr.urg_ptr = cpu_to_be16(9865);

	return store_bits(skb, offset, &hdr, sizeof(hdr));
}

#define ICMP4_HDR_LEN sizeof(struct icmphdr)
static int init_icmp4_hdr_info(struct sk_buff *skb, int *offset,
		__u16 src, __u16 dst, u16 datagram_len)
{
	struct icmphdr hdr;

	hdr.type = ICMP_ECHO;
	hdr.code = 0;
	hdr.checksum = 0;
	hdr.un.echo.id = cpu_to_be16(src);
	hdr.un.echo.sequence = cpu_to_be16(2000);

	return store_bits(skb, offset, &hdr, sizeof(hdr));
}

int init_icmp4_hdr_error(struct sk_buff *skb, int *offset, __u16 src, __u16 dst,
		u16 datagram_len)
{
	struct icmphdr hdr;

	hdr.type = ICMP_DEST_UNREACH;
	hdr.code = ICMP_FRAG_NEEDED;
	hdr.checksum = 0;
	hdr.un.frag.__unused = cpu_to_be16(0);
	hdr.un.frag.mtu = cpu_to_be16(1500);

	return store_bits(skb, offset, &hdr, sizeof(hdr));
}

#define ICMP6_HDR_LEN sizeof(struct icmp6hdr)
static int init_icmp6_hdr_info(struct sk_buff *skb, int *offset,
		__u16 src, __u16 dst, u16 datagram_len)
{
	struct icmp6hdr hdr;

	hdr.icmp6_type = ICMPV6_ECHO_REQUEST;
	hdr.icmp6_code = 0;
	hdr.icmp6_cksum = 0;
	hdr.icmp6_dataun.u_echo.identifier = cpu_to_be16(src);
	hdr.icmp6_dataun.u_echo.sequence = cpu_to_be16(2000);

	return store_bits(skb, offset, &hdr, sizeof(hdr));
}

int init_icmp6_hdr_error(struct sk_buff *skb, int *offset, __u16 src, __u16 dst,
		u16 datagram_len)
{
	struct icmp6hdr hdr;

	hdr.icmp6_type = ICMPV6_PKT_TOOBIG;
	hdr.icmp6_code = 0;
	hdr.icmp6_cksum = 0;
	hdr.icmp6_mtu = cpu_to_be32(1500);

	return store_bits(skb, offset, &hdr, sizeof(hdr));
}

int init_payload_normal(struct sk_buff *skb, int *offset)
{
	unsigned char buffer[256];
	unsigned int i;
	int error;

	for (i = 0; i < sizeof(buffer); i++)
		buffer[i] = i;

	for (i = (*offset); i < skb->len;) {
		error = store_bits(skb, &i, buffer, sizeof(buffer));
		if (error)
			return error;
	}

	*offset = i;
	return 0;
}

static int init_payload_inner_ipv6(struct sk_buff *skb, int *offset)
{
	int error;

	error = init_ipv6_hdr(skb, offset, "1::1", "64::192.0.2.5", 1300,
			NEXTHDR_TCP, 32);
	if (error)
		return error;

	error = init_tcp_hdr(skb, offset, 50080, 51234, 1300);
	if (error)
		return error;

	return init_payload_normal(skb, offset);
}

static int init_payload_inner_ipv4(struct sk_buff *skb, int *offset)
{
	int error;

	error = init_ipv4_hdr(skb, offset, "192.0.2.128", "192.0.2.2", 1300,
			IPPROTO_TCP, 32);
	if (error)
		return error;

	error = init_tcp_hdr(skb, offset, 1024, 80, 1300);
	if (error)
		return error;

	return init_payload_normal(skb, offset);
}

static int ipv4_tcp_post(struct sk_buff *skb)
{
	struct iphdr *hdr;
	unsigned int len;

	if (HDR4_LEN > skb->len)
		return 0;

	hdr = ip_hdr(skb);
	len = skb->len - HDR4_LEN;
	tcp_hdr(skb)->check = csum_tcpudp_magic(hdr->saddr, hdr->daddr, len,
			IPPROTO_TCP, skb_checksum(skb, HDR4_LEN, len, 0));

	return 0;
}

static int ipv4_udp_post(struct sk_buff *skb)
{
	struct iphdr *hdr4;
	struct udphdr *uhdr;
	unsigned int len;

	if (HDR4_LEN > skb->len)
		return 0;

	hdr4 = ip_hdr(skb);
	uhdr = udp_hdr(skb);
	len = skb->len - HDR4_LEN;
	uhdr->check = csum_tcpudp_magic(hdr4->saddr, hdr4->daddr, len,
			IPPROTO_UDP, skb_checksum(skb, HDR4_LEN, len, 0));
	if (uhdr->check == 0)
		uhdr->check = CSUM_MANGLED_0;

	return 0;
}

static int ipv4_icmp_post(struct sk_buff *skb)
{
	/* hdr->checksum = ip_compute_csum(...); */
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	return 0;
}

static int ipv6_tcp_post(struct sk_buff *skb)
{
	struct ipv6hdr *hdr;
	unsigned int len;

	if (HDR6_LEN > skb->len)
		return 0;

	hdr = ipv6_hdr(skb);
	len = skb->len - HDR6_LEN;
	tcp_hdr(skb)->check = csum_ipv6_magic(&hdr->saddr, &hdr->daddr,
			len, NEXTHDR_TCP, skb_checksum(skb, HDR6_LEN, len, 0));

	return 0;
}

static int ipv6_udp_post(struct sk_buff *skb)
{
	struct ipv6hdr *hdr;
	unsigned int len;

	if (HDR6_LEN > skb->len)
		return 0;

	hdr = ipv6_hdr(skb);
	len = skb->len - HDR6_LEN;
	udp_hdr(skb)->check = csum_ipv6_magic(&hdr->saddr, &hdr->daddr, len,
			NEXTHDR_UDP, skb_checksum(skb, HDR6_LEN, len, 0));

	return 0;
}

static int ipv6_icmp_post(struct sk_buff *skb)
{
	struct ipv6hdr *hdr;
	unsigned int len;

	if (HDR6_LEN > skb->len)
		return 0;

	hdr = ipv6_hdr(skb);
	len = skb->len - HDR6_LEN;
	icmp6_hdr(skb)->icmp6_cksum = csum_ipv6_magic(&hdr->saddr, &hdr->daddr,
			len, NEXTHDR_ICMP, skb_checksum(skb, HDR6_LEN, len, 0));

	return 0;
}

typedef int (*l3_hdr_cb)(struct sk_buff *, int *, char *, char *, u16, u8, u8);
typedef int (*l4_hdr_cb)(struct sk_buff *, int *, __u16, __u16, u16);
typedef int (*payload_cb)(struct sk_buff *, int *);
typedef int (*l4_post_cb)(struct sk_buff *);

struct proto_meta {
	union {
		l3_hdr_cb l3;
		l4_hdr_cb l4;
	} init_cb;
	int hdr_type;
	int hdr_len;
};

static struct proto_meta ipv6_meta = {
	.init_cb.l3 = init_ipv6_hdr,
	.hdr_type = ETH_P_IPV6,
	.hdr_len = HDR6_LEN,
};
static struct proto_meta ipv4_meta = {
	.init_cb.l3 = init_ipv4_hdr,
	.hdr_type = ETH_P_IP,
	.hdr_len = HDR4_LEN,
};
static struct proto_meta tcp_meta = {
	.init_cb.l4 = init_tcp_hdr,
	.hdr_type = NEXTHDR_TCP,
	.hdr_len = TCP_HDR_LEN
};
static struct proto_meta udp_meta = {
	.init_cb.l4 = init_udp_hdr,
	.hdr_type = NEXTHDR_UDP,
	.hdr_len = UDP_HDR_LEN
};
static struct proto_meta icmp6info_meta = {
	.init_cb.l4 = init_icmp6_hdr_info,
	.hdr_type = NEXTHDR_ICMP,
	.hdr_len = ICMP6_HDR_LEN,
};
static struct proto_meta icmp6err_meta =  {
	.init_cb.l4 = init_icmp6_hdr_error,
	.hdr_type = NEXTHDR_ICMP,
	.hdr_len = ICMP6_HDR_LEN,
};
static struct proto_meta icmp4info_meta = {
	.init_cb.l4 = init_icmp4_hdr_info,
	.hdr_type = IPPROTO_ICMP,
	.hdr_len = ICMP4_HDR_LEN,
};
static struct proto_meta icmp4err_meta =  {
	.init_cb.l4 = init_icmp4_hdr_error,
	.hdr_type = IPPROTO_ICMP,
	.hdr_len = ICMP4_HDR_LEN,
};

static int create_skb(struct proto_meta *l3, char *src, char *dst, u8 ttl,
		struct proto_meta *l4, __u16 src_port, __u16 dst_port,
		payload_cb payload_fn, u16 payload_len,
		l4_post_cb l4_post_fn,
		struct sk_buff **result)
{
	struct sk_buff *skb;
	int dlen = l4->hdr_len + payload_len; /* Datagram length */
	int offset = 0;
	int error;

	skb = alloc_skb(LL_MAX_HEADER + l3->hdr_len + dlen, GFP_ATOMIC);
	if (!skb) {
		log_err("New packet allocation failed.");
		return -ENOMEM;
	}
	skb->protocol = htons(l3->hdr_type);

	skb_reserve(skb, LL_MAX_HEADER);
	skb_put(skb, l3->hdr_len + l4->hdr_len + payload_len);

	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	skb_set_transport_header(skb, l3->hdr_len);

	error = l3->init_cb.l3(skb, &offset, src, dst, dlen, l4->hdr_type, ttl);
	if (error)
		goto failure;
	error = l4->init_cb.l4(skb, &offset, src_port, dst_port, dlen);
	if (error)
		goto failure;
	error = payload_fn(skb, &offset);
	if (error)
		goto failure;
	error = l4_post_fn(skb);
	if (error)
		goto failure;

	*result = skb;
	return 0;

failure:
	kfree_skb(skb);
	return error;
}

int create_skb6_udp(char *saddr, __u16 sport, char *daddr, __u16 dport,
		u16 payload_len, u8 ttl, struct sk_buff **result)
{
	return create_skb(&ipv6_meta, saddr, daddr, ttl,
			&udp_meta, sport, dport,
			init_payload_normal, payload_len,
			ipv6_udp_post,
			result);
}

int create_skb6_tcp(char *saddr, __u16 sport, char *daddr, __u16 dport,
		u16 payload_len, u8 ttl, struct sk_buff **result)
{
	return create_skb(&ipv6_meta, saddr, daddr, ttl,
			&tcp_meta, sport, dport,
			init_payload_normal, payload_len,
			ipv6_tcp_post,
			result);
}

int create_skb6_icmp_info(char *saddr, char *daddr, __u16 id,
		u16 payload_len, u8 ttl, struct sk_buff **result)
{
	return create_skb(&ipv6_meta, saddr, daddr, ttl,
			&icmp6info_meta, id, 0,
			init_payload_normal, payload_len,
			ipv6_icmp_post,
			result);
}

int create_skb6_icmp_error(char *saddr, char *daddr,
		u16 payload_len, u8 ttl, struct sk_buff **result)
{
	return create_skb(&ipv6_meta, saddr, daddr, ttl,
			&icmp6err_meta, 0, 0,
			init_payload_inner_ipv6, payload_len,
			ipv6_icmp_post,
			result);
}

int create_skb4_udp(char *saddr, __u16 sport, char *daddr, __u16 dport,
		u16 payload_len, u8 ttl, struct sk_buff **result)
{
	return create_skb(&ipv4_meta, saddr, daddr, ttl,
			&udp_meta, sport, dport,
			init_payload_normal, payload_len,
			ipv4_udp_post,
			result);
}

int create_skb4_tcp(char *saddr, __u16 sport, char *daddr, __u16 dport,
		u16 payload_len, u8 ttl, struct sk_buff **result)
{
	return create_skb(&ipv4_meta, saddr, daddr, ttl,
			&tcp_meta, sport, dport,
			init_payload_normal, payload_len,
			ipv4_tcp_post,
			result);
}

int create_skb4_icmp_info(char *saddr, char *daddr, __u16 id,
		u16 payload_len, u8 ttl, struct sk_buff **result)
{
	return create_skb(&ipv4_meta, saddr, daddr, ttl,
			&icmp4info_meta, id, 0,
			init_payload_normal, payload_len,
			ipv4_icmp_post,
			result);
}

int create_skb4_icmp_error(char *saddr, char *daddr,
		u16 payload_len, u8 ttl, struct sk_buff **result)
{
	return create_skb(&ipv4_meta, saddr, daddr, ttl,
			&icmp4err_meta, 0, 0,
			init_payload_inner_ipv4, payload_len,
			ipv4_icmp_post,
			result);
}

int create_tcp_packet(struct sk_buff **skb, l3_protocol l3_proto, bool syn, bool rst, bool fin)
{
	struct tcphdr *hdr_tcp;
	int error;

	switch (l3_proto) {
	case L3PROTO_IPV4:
		error = create_skb4_tcp("8.7.6.5", 8765, "5.6.7.8", 5678,
				100, 32, skb);
		if (error)
			return error;
		break;
	case L3PROTO_IPV6:
		error = create_skb6_tcp("1::2", 1212, "3::4", 3434,
				100, 32, skb);
		if (error)
			return error;
		break;
	}

	hdr_tcp = tcp_hdr(*skb);
	hdr_tcp->syn = syn;
	hdr_tcp->rst = rst;
	hdr_tcp->fin = fin;

	return 0;
}
