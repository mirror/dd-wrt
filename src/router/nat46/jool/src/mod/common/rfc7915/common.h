#ifndef SRC_MOD_COMMON_RFC7915_COMMON_H_
#define SRC_MOD_COMMON_RFC7915_COMMON_H_

#include <linux/ip.h>
#include "common/types.h"
#include "mod/common/packet.h"
#include "mod/common/translation_state.h"

/**
 * An accesor for the full unused portion of the ICMP header, which I feel is
 * missing from linux/icmp.h.
 */
#define icmp4_unused un.gateway

typedef verdict (*skb_alloc_fn)(struct xlation *);
typedef verdict (*header_xlat_fn)(struct xlation *);

struct translation_steps {
	/**
	 * Routes the outgoing packet, allocates it, then copies dst_entry and
	 * layer 4 payload into it. Ensures there's enough headroom (bytes
	 * between skb->head and skb->data) for translated headers.
	 * (In other words, it does everything except for headers.)
	 *
	 * "Why do we need this? Why don't we simply override the headers of the
	 * incoming packet? This would avoid lots of allocation and copying."
	 *
	 * Because we can't afford to completely lose the original headers until
	 * we've fetched the translated packet successfully. Even after the
	 * RFC7915 code ends, there is still stuff we might need the original
	 * packet for, such as replying an ICMP error or NF_ACCEPTing.
	 *
	 * There's also the issue that the incoming packet might not have enough
	 * room for the header length expansion from v4 to v6.
	 */
	skb_alloc_fn skb_alloc;
	/** The function that will translate the external IP header. */
	header_xlat_fn xlat_outer_l3;
	/**
	 * The function that will translate the internal IP header.
	 * (ICMP errors only.)
	 */
	header_xlat_fn xlat_inner_l3;
	/**
	 * Translates everything between the external IP header and the L4
	 * payload.
	 */
	header_xlat_fn xlat_tcp;
	header_xlat_fn xlat_udp;
	header_xlat_fn xlat_icmp;
};

void partialize_skb(struct sk_buff *skb, __u16 csum_offset);
bool will_need_frag_hdr(const struct iphdr *hdr);
verdict ttpcomm_translate_inner_packet(struct xlation *state,
		struct translation_steps const *steps);

struct bkp_skb {
	unsigned int pulled;
	struct {
		int l3;
		int l4;
	} offset;
	unsigned int payload;
	l4_protocol l4_proto;
};

struct bkp_skb_tuple {
	struct bkp_skb in;
	struct bkp_skb out;
};

verdict become_inner_packet(struct xlation *state, struct bkp_skb_tuple *bkp,
		bool do_out);
void restore_outer_packet(struct xlation *state, struct bkp_skb_tuple *bkp,
		bool do_out);

verdict xlat_l4_function(struct xlation *state,
		struct translation_steps const *steps);

bool must_not_translate(struct in_addr *addr, struct net *ns);

/* ICMP Extensions */

#define icmp6_length icmp6_dataun.un_data8[0]
/* un.reserved does not exist in old kernels. */
#define icmp4_length(hdr) (((__u8 *)(&(hdr)->un.gateway))[1])

/* See /test/graybox/test-suite/siit/7915/README.md#ic */
struct icmpext_args {
	size_t max_pkt_len; /* Maximum (allowed outgoing) Packet Length */
	size_t ipl; /* Internal Packet Length */
	size_t out_bits; /* 4->6: Set as 3; 6->4: Set as 2 */
	bool force_remove_ie; /* Force removal of ICMP Extension? */
};

verdict handle_icmp_extension(struct xlation *state,
		struct icmpext_args *args);

void skb_cleanup_copy(struct sk_buff *skb);

#endif /* SRC_MOD_COMMON_RFC7915_COMMON_H_ */
