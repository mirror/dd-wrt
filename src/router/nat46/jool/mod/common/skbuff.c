#include "mod/common/skbuff.h"

#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/printk.h>
#include <net/ipv6.h>

#include "mod/common/linux_version.h"
#include "mod/common/packet.h"

enum hdr_protocol {
	HP_IPV4 = 4,
	HP_IPV6 = 128,
	HP_TCP = NEXTHDR_TCP,
	HP_UDP = NEXTHDR_UDP,
	HP_ICMP4 = IPPROTO_ICMP,
	HP_ICMP6 = NEXTHDR_ICMP,
	HP_HOP = NEXTHDR_HOP,
	HP_ROUTING = NEXTHDR_ROUTING,
	HP_DEST = NEXTHDR_DEST,
	HP_FRAG = NEXTHDR_FRAGMENT,
	HP_PAYLOAD = 129,
};

struct hdr_iterator {
	unsigned int tabs;
	struct sk_buff *skb;
	int skb_offset;
	enum hdr_protocol type;
	bool done;
};

#define skb_hdr_ptr(iterator, buffer) skb_header_pointer(iterator->skb, \
		iterator->skb_offset, sizeof(buffer), &buffer)

#define print(tabs, text, ...) do { \
		print_tabs(tabs); \
		pr_cont(text "\n", ##__VA_ARGS__); \
	} while (0)

static void __skb_log(struct sk_buff *skb, char *header, unsigned int tabs);

static void print_tabs(unsigned int tabs)
{
	unsigned int t;

	pr_info("");
	for (t = 0; t < tabs; t++)
		pr_cont("    ");
}

static char *skbproto2string(__u16 proto)
{
	switch (proto) {
	case ETH_P_IP:
		return "IPv4";
	case ETH_P_IPV6:
		return "IPv6";
	}

	return "unknown";
}

static char *ipsummed2string(__u8 ip_summed)
{
	switch (ip_summed) {
	case CHECKSUM_NONE:
		return "CHECKSUM_NONE";
	case CHECKSUM_UNNECESSARY:
		return "CHECKSUM_UNNECESSARY";
	case CHECKSUM_COMPLETE:
		return "CHECKSUM_COMPLETE";
	case CHECKSUM_PARTIAL:
		return "CHECKSUM_PARTIAL";
	}

	return "unknown";
}

static void print_skb_fields(struct sk_buff *skb, unsigned int tabs)
{
	__u16 proto;

	print(tabs++, "skb fields:");
	print(tabs, "prev:%p", skb->prev);
	print(tabs, "next:%p", skb->next);
	print(tabs, "dev:%p", skb->dev);
	print(tabs, "len:%u", skb->len);
	print(tabs, "data_len:%u", skb->data_len);
	print(tabs, "mac_len:%u", skb->mac_len);
	print(tabs, "hdr_len:%u", skb->hdr_len);
	print(tabs, "truesize:%u", skb->truesize);
	print(tabs, "pkt_type:%u", skb->pkt_type);
	print(tabs, "ignore_df:%u", skb->ignore_df);
	print(tabs, "ip_summed:%u (%s)", skb->ip_summed,
			ipsummed2string(skb->ip_summed));
	print(tabs, "csum_valid:%u", skb->csum_valid);
	print(tabs, "csum_start:%u", skb->csum_start);
	print(tabs, "csum_offset:%u", skb->csum_offset);
	print(tabs, "mark:%u", skb->mark);
	print(tabs, "inner_transport_header:%u", skb->inner_transport_header);
	print(tabs, "inner_network_header:%u", skb->inner_network_header);
	print(tabs, "inner_mac_header:%u", skb->inner_mac_header);

	proto = be16_to_cpu(skb->protocol);
	print(tabs, "protocol:%u (%s)", proto, skbproto2string(proto));

	print(tabs, "transport_header:%u", skb->transport_header);
	print(tabs, "network_header:%u", skb->network_header);
	print(tabs, "mac_header:%u", skb->mac_header);
	print(tabs, "head:%p", skb->head);
	print(tabs, "data:%ld", skb->data - skb->head);
	print(tabs, "tail:%u", skb->tail);
	print(tabs, "end:%u", skb->end);
}

static int truncated(unsigned int tabs)
{
	print(tabs, "[Truncated]");
	return -ENOSPC;
}

static int print_ipv4_hdr(struct hdr_iterator *meta)
{
	struct iphdr buffer, *hdr;
	unsigned int tabs = meta->tabs;

	print(tabs++, "IPv4 header:");

	hdr = skb_hdr_ptr(meta, buffer);
	if (!hdr)
		return truncated(tabs);

	print(tabs, "Version: %u", hdr->version);
	print(tabs, "IHL: %u", hdr->ihl);
	print(tabs, "TOS: %u", hdr->tos);
	print(tabs, "Total Length: %u", be16_to_cpu(hdr->tot_len));
	print(tabs, "Fragment ID: %u", be16_to_cpu(hdr->id));
	print(tabs, "Reserved:%u DF:%u MF:%u FragmentOffset: %u",
			be16_to_cpu(hdr->frag_off) >> 15,
			!!is_df_set(hdr),
			!!is_mf_set_ipv4(hdr),
			get_fragment_offset_ipv4(hdr));
	print(tabs, "TTL: %u", hdr->ttl);
	print(tabs, "Protocol: %u", hdr->protocol);
	print(tabs, "Checksum: 0x%x", be16_to_cpu(hdr->check));
	print(tabs, "Source Address: %pI4", &hdr->saddr);
	print(tabs, "Destination Address: %pI4", &hdr->daddr);

	meta->skb_offset += sizeof(buffer);
	meta->type = is_first_frag4(hdr) ? hdr->protocol : HP_PAYLOAD;
	return 0;
}

static int print_ipv6_hdr(struct hdr_iterator *meta)
{
	struct ipv6hdr buffer, *hdr;
	unsigned int tabs = meta->tabs;

	print(tabs++, "IPv6 header:");

	hdr = skb_hdr_ptr(meta, buffer);
	if (!hdr)
		return truncated(tabs);

	print(tabs, "Version: %u", hdr->version);
	print(tabs, "Priority: %u", hdr->priority);
	print(tabs, "Flow Label: 0x%02x%02x%02x", hdr->flow_lbl[0],
			hdr->flow_lbl[1], hdr->flow_lbl[2]);
	print(tabs, "Payload Length: %u", be16_to_cpu(hdr->payload_len));
	print(tabs, "Next Header: %u", hdr->nexthdr);
	print(tabs, "Hop Limit: %u", hdr->hop_limit);
	print(tabs, "Source Address: %pI6c", &hdr->saddr);
	print(tabs, "Destination Address: %pI6c", &hdr->daddr);

	meta->skb_offset += sizeof(buffer);
	meta->type = hdr->nexthdr;
	return 0;
}

static int print_tcphdr(struct hdr_iterator *meta)
{
	struct tcphdr buffer, *hdr;
	unsigned int tabs = meta->tabs;

	print(tabs++, "TCP header:");

	hdr = skb_hdr_ptr(meta, buffer);
	if (!hdr)
		return truncated(tabs);

	print(tabs, "Src Port: %u", be16_to_cpu(hdr->source));
	print(tabs, "Dst Port: %u", be16_to_cpu(hdr->dest));
	print(tabs, "Seq Number: %u", be32_to_cpu(hdr->seq));
	print(tabs, "ACK Seq: %u", be32_to_cpu(hdr->ack_seq));
	print(tabs, "ACK:%u RST:%u SYN:%u FIN:%u",
			hdr->ack, hdr->rst, hdr->syn, hdr->fin);
	print(tabs, "[Other flags ommitted]");
	print(tabs, "Window Size: %u", be16_to_cpu(hdr->window));
	print(tabs, "Checksum: 0x%x", be16_to_cpu(hdr->check));
	print(tabs, "Urgent Pointer: %u", be16_to_cpu(hdr->urg_ptr));

	meta->skb_offset += sizeof(buffer);
	meta->type = HP_PAYLOAD;
	return 0;
}

static int print_udphdr(struct hdr_iterator *meta)
{
	struct udphdr buffer, *hdr;
	unsigned int tabs = meta->tabs;

	print(tabs++, "UDP header:");

	hdr = skb_hdr_ptr(meta, buffer);
	if (!hdr)
		return truncated(tabs);

	print(tabs, "Src Port: %u", be16_to_cpu(hdr->source));
	print(tabs, "Dst Port: %u", be16_to_cpu(hdr->dest));
	print(tabs, "Length: %u", be16_to_cpu(hdr->len));
	print(tabs, "Checksum: 0x%x", be16_to_cpu(hdr->check));

	meta->skb_offset += sizeof(buffer);
	meta->type = HP_PAYLOAD;
	return 0;
}

static int print_icmp4hdr(struct hdr_iterator *meta)
{
	struct icmphdr buffer, *hdr;
	unsigned int tabs = meta->tabs;

	print(tabs++, "ICMPv4 header:");

	hdr = skb_hdr_ptr(meta, buffer);
	if (!hdr)
		return truncated(tabs);

	print(tabs, "Type:%u Code:%u", hdr->type, hdr->code);
	print(tabs, "Checksum: 0x%x", be16_to_cpu(hdr->checksum));
	print(tabs, "Rest 1: %u", be16_to_cpu(hdr->un.echo.id));
	print(tabs, "Rest 2: %u", be16_to_cpu(hdr->un.echo.sequence));

	meta->skb_offset += sizeof(buffer);
	meta->type = is_icmp4_error(hdr->type) ? HP_IPV4 : HP_PAYLOAD;
	return 0;
}

static int print_icmp6hdr(struct hdr_iterator *meta)
{
	struct icmp6hdr buffer, *hdr;
	unsigned int tabs = meta->tabs;

	print(tabs++, "ICMPv6 header:");

	hdr = skb_hdr_ptr(meta, buffer);
	if (!hdr)
		return truncated(tabs);

	print(tabs, "Type:%u Code:%u", hdr->icmp6_type, hdr->icmp6_code);
	print(tabs, "Checksum: 0x%x", be16_to_cpu(hdr->icmp6_cksum));
	print(tabs, "Rest 1: %u", be16_to_cpu(hdr->icmp6_identifier));
	print(tabs, "Rest 2: %u", be16_to_cpu(hdr->icmp6_sequence));

	meta->skb_offset += sizeof(buffer);
	meta->type = is_icmp6_error(hdr->icmp6_type) ? HP_IPV6 : HP_PAYLOAD;
	return 0;
}

static int print_exthdr(struct hdr_iterator *meta)
{
	struct ipv6_opt_hdr buffer, *hdr;
	unsigned int tabs = meta->tabs;
	unsigned int len;

	print(tabs++, "IPv6 Extension header:");

	hdr = skb_hdr_ptr(meta, buffer);
	if (!hdr)
		return truncated(tabs);

	len = 8 + 8 * hdr->hdrlen;

	print(tabs, "Next Header: %u", hdr->nexthdr);
	print(tabs, "Length: %u", len);

	meta->skb_offset += len;
	meta->type = hdr->nexthdr;
	return 0;
}

static int print_fraghdr(struct hdr_iterator *meta)
{
	struct frag_hdr buffer, *hdr;
	unsigned int tabs = meta->tabs;

	print(tabs++, "Fragment Header:");

	hdr = skb_hdr_ptr(meta, buffer);
	if (!hdr)
		return truncated(tabs);

	print(tabs, "Next Header: %u", hdr->nexthdr);
	print(tabs, "Reserved: %u", hdr->reserved);
	print(tabs, "FragmentOffset:%u Res:%u M:%u",
			get_fragment_offset_ipv6(hdr),
			(be16_to_cpu(hdr->frag_off) >> 1) & 3,
			is_mf_set_ipv6(hdr));
	print(tabs, "Identification: %u", be32_to_cpu(hdr->identification));

	meta->skb_offset += sizeof(buffer);
	meta->type = is_first_frag6(hdr) ? hdr->nexthdr : HP_PAYLOAD;
	return 0;
}

static int print_payload(struct hdr_iterator *meta)
{
	unsigned char buffer[10], *payload;
	unsigned int payload_len;
	unsigned int tabs = meta->tabs;
	unsigned int i;

	print(tabs++, "Payload:");

	if (meta->skb->len < meta->skb_offset) {
		print(tabs, "Error: Offset exceeds packet length.");
		return -ENOSPC;
	}

	payload_len = meta->skb->len - meta->skb_offset;

	if (payload_len == 0) {
		print(tabs, "(Empty)");
		goto success;
	}

	/*
	 * Most (if not all) test skb payloads are monotonic (0, 1, 2, 3, 4,
	 * 5, ..., 254, 255, 0, 1, 2, 3, ..., 254, 255, 0, 1, etc) and it's
	 * rare to need to confirm large payloads visually.
	 * So let's print only a few bytes.
	 */
	if (payload_len <= 10) {
		payload = skb_header_pointer(meta->skb, meta->skb_offset,
				payload_len, buffer);
		if (!payload)
			return truncated(tabs);
		print_tabs(tabs);
		for (i = 0; i < payload_len; i++)
			pr_cont("%02x ", payload[i]);

	} else {
		payload = skb_header_pointer(meta->skb, meta->skb_offset, 5,
				buffer);
		if (!payload)
			return truncated(tabs);
		print_tabs(tabs);
		for (i = 0; i < 5; i++)
			pr_cont("%02x ", payload[i]);

		pr_cont("(...) ");

		payload = skb_header_pointer(meta->skb, meta->skb->len - 5, 5,
				buffer);
		if (!payload)
			return truncated(tabs);
		for (i = 0; i < 5; i++)
			pr_cont("%02x ", payload[i]);
	}

	pr_cont("\n");
success:
	meta->done = true;
	return 0;
}

static void print_hdr_chain(struct hdr_iterator *meta)
{
	int error;

	while (!meta->done) {
		switch (meta->type) {
		case HP_IPV4:
			error = print_ipv4_hdr(meta);
			break;
		case HP_IPV6:
			error = print_ipv6_hdr(meta);
			break;
		case HP_TCP:
			error = print_tcphdr(meta);
			break;
		case HP_UDP:
			error = print_udphdr(meta);
			break;
		case HP_ICMP4:
			error = print_icmp4hdr(meta);
			break;
		case HP_ICMP6:
			error = print_icmp6hdr(meta);
			break;
		case HP_HOP:
		case HP_ROUTING:
		case HP_DEST:
			error = print_exthdr(meta);
			break;
		case HP_FRAG:
			error = print_fraghdr(meta);
			break;
		case HP_PAYLOAD:
			error = print_payload(meta);
			break;
		default:
			print(meta->tabs, "[Unknown header type: %u]",
					meta->type);
			return;
		}

		if (error)
			return;
	}
}

static void print_headers(struct sk_buff *skb, unsigned int tabs)
{
	struct hdr_iterator meta = {
		.tabs = tabs + 1,
		.skb = skb,
		.skb_offset = skb_network_offset(skb),
		.type = skb->data[0] >> 4,
		.done = false,
	};

	print(tabs, "Content:");
	switch (meta.type) {
	case 4:
		meta.type = HP_IPV4;
		break;
	case 6:
		meta.type = HP_IPV6;
		break;
	default:
		print(tabs + 1, "[Unknown layer 3 protocol: %u]", meta.type);
		return;
	}

	print_hdr_chain(&meta);
}

static void print_shinfo_fields(struct sk_buff *skb, unsigned int tabs)
{
	struct skb_shared_info *shinfo = skb_shinfo(skb);
	unsigned int f;
	struct sk_buff *iter;

	print(tabs, "shared info:");
	tabs++;

	print(tabs, "nr_frags:%u", shinfo->nr_frags);
	print(tabs, "tx_flags:%u", shinfo->tx_flags);
	print(tabs, "gso_size:%u", shinfo->gso_size);
	print(tabs, "gso_segs:%u", shinfo->gso_segs);
	print(tabs, "gso_type:%u", shinfo->gso_type);

	print(tabs, "frags:");
	tabs++;
	for (f = 0; f < shinfo->nr_frags; f++) {
		print(tabs, "%u page_offset:%u size:%u", f,
#if LINUX_VERSION_AT_LEAST(5, 4, 0, 9, 0)
				skb_frag_off(&shinfo->frags[f]),
#else
				shinfo->frags[f].page_offset,
#endif
				skb_frag_size(&shinfo->frags[f]));
	}
	tabs--;

	skb_walk_frags(skb, iter)
		__skb_log(iter, "frag", tabs);
}

static void __skb_log(struct sk_buff *skb, char *header, unsigned int tabs)
{
	pr_info("=================\n");
	print(tabs, "%s", header);
	tabs++;

	print_skb_fields(skb, tabs);
	print_headers(skb, tabs);
	print_shinfo_fields(skb, tabs);
}

/**
 * Assumes that the headers of the packet can be found in the head area.
 * (ie. Do not call before `pkt_init_ipv*()`.)
 */
void skb_log(struct sk_buff *skb, char *label)
{
	__skb_log(skb, label, 0);
}
