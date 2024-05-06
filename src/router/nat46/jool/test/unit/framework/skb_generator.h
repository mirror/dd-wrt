#ifndef _JOOL_UNIT_SKB_GENERATOR_H
#define _JOOL_UNIT_SKB_GENERATOR_H

#include "common/types.h"
#include "mod/common/packet.h"

int init_ipv4_hdr(struct sk_buff *skb, int *offset, char *src, char *dst,
		u16 payload_len, u8 nexthdr, u8 ttl);
int init_ipv6_hdr(struct sk_buff *skb, int *offset, char *src, char *dst,
		u16 payload_len, u8 nexthdr, u8 ttl);

int init_tcp_hdr(struct sk_buff *skb, int *offset, __u16 src, __u16 dst,
		u16 datagram_len);
int init_icmp6_hdr_error(struct sk_buff *skb, int *offset, __u16 src, __u16 dst,
		u16 datagram_len);
int init_icmp4_hdr_error(struct sk_buff *skb, int *offset, __u16 src, __u16 dst,
		u16 datagram_len);
int init_payload_normal(struct sk_buff *skb, int *offset);

int create_skb6_udp(char *saddr, __u16 sport, char *daddr, __u16 dport,
		u16 payload_len, u8 ttl, struct sk_buff **result);
int create_skb6_tcp(char *saddr, __u16 sport, char *daddr, __u16 dport,
		u16 payload_len, u8 ttl, struct sk_buff **result);
int create_skb6_icmp_info(char *saddr, char *daddr, __u16 id,
		u16 payload_len, u8 ttl, struct sk_buff **result);
int create_skb6_icmp_error(char *saddr, char *daddr,
		u16 payload_len, u8 ttl, struct sk_buff **result);

int create_skb4_udp(char *saddr, __u16 sport, char *daddr, __u16 dport,
		u16 payload_len, u8 ttl, struct sk_buff **result);
int create_skb4_tcp(char *saddr, __u16 sport, char *daddr, __u16 dport,
		u16 payload_len, u8 ttl, struct sk_buff **result);
int create_skb4_icmp_info(char *saddr, char *daddr, __u16 id,
		u16 payload_len, u8 ttl, struct sk_buff **result);
int create_skb4_icmp_error(char *saddr, char *daddr,
		u16 payload_len, u8 ttl, struct sk_buff **result);

int create_tcp_packet(struct sk_buff **skb, l3_protocol l3_proto,
		bool syn, bool rst, bool fin);

#endif /* _JOOL_UNIT_SKB_GENERATOR_H */
