#include "mod/common/rfc7915/common.h"

#include <linux/icmp.h>
#include "common/config.h"
#include "mod/common/ipv6_hdr_iterator.h"
#include "mod/common/linux_version.h"
#include "mod/common/log.h"
#include "mod/common/packet.h"
#include "mod/common/stats.h"
#include "mod/common/db/denylist4.h"
#include "mod/common/steps/compute_outgoing_tuple.h"

/**
 * Note: Fragmentation offloading is handled transparently: NIC joins fragments,
 * we translate the large and seemingly unfragmented packet, then NIC fragments
 * again, re-adding the fragment header.
 *
 * Same happens with defrag: Defrag defrags, Jool translates seemingly
 * unfragmented, enfrag enfrags.
 *
 * This function only returns true when WE are supposed to worry about the
 * fragment header. (ie. we're translating a completely unmanhandled fragment.)
 */
bool will_need_frag_hdr(const struct iphdr *hdr)
{
	return is_fragmented_ipv4(hdr);
}

static int move_pointers_in(struct packet *pkt, __u8 protocol,
		unsigned int l3hdr_len)
{
	unsigned int l4hdr_len;

	if (!jskb_pull(pkt->skb, pkt_hdrs_len(pkt)))
		return -EINVAL;
	skb_reset_network_header(pkt->skb);
	skb_set_transport_header(pkt->skb, l3hdr_len);

	switch (protocol) {
	case IPPROTO_TCP:
		pkt->l4_proto = L4PROTO_TCP;
		l4hdr_len = tcp_hdr_len(pkt_tcp_hdr(pkt));
		break;
	case IPPROTO_UDP:
		pkt->l4_proto = L4PROTO_UDP;
		l4hdr_len = sizeof(struct udphdr);
		break;
	case IPPROTO_ICMP:
	case NEXTHDR_ICMP:
		pkt->l4_proto = L4PROTO_ICMP;
		l4hdr_len = sizeof(struct icmphdr);
		break;
	default:
		pkt->l4_proto = L4PROTO_OTHER;
		l4hdr_len = 0;
		break;
	}
	pkt->is_inner = true;
	pkt->payload_offset = skb_transport_offset(pkt->skb) + l4hdr_len;

	return 0;
}

static int move_pointers_out(struct packet *in, struct packet *out,
		unsigned int l3hdr_len)
{
	if (!jskb_pull(out->skb, pkt_hdrs_len(out)))
		return -EINVAL;
	skb_reset_network_header(out->skb);
	skb_set_transport_header(out->skb, l3hdr_len);

	out->l4_proto = pkt_l4_proto(in);
	out->is_inner = true;
	out->payload_offset = skb_transport_offset(out->skb)
			+ pkt_l4hdr_len(in);

	return 0;
}

static int move_pointers4(struct packet *in, struct packet *out, bool do_out)
{
	struct iphdr *hdr4;
	unsigned int l3hdr_len;
	int error;

	hdr4 = pkt_payload(in);
	error = move_pointers_in(in, hdr4->protocol, 4 * hdr4->ihl);
	if (error)
		return error;

	if (!do_out)
		return 0;

	l3hdr_len = sizeof(struct ipv6hdr);
	if (will_need_frag_hdr(hdr4))
		l3hdr_len += sizeof(struct frag_hdr);
	return move_pointers_out(in, out, l3hdr_len);
}

static int move_pointers6(struct packet *in, struct packet *out, bool do_out)
{
	struct ipv6hdr *hdr6 = pkt_payload(in);
	struct hdr_iterator iterator = HDR_ITERATOR_INIT(hdr6);
	int error;

	hdr_iterator_last(&iterator);

	error = move_pointers_in(in, iterator.hdr_type,
			iterator.data - (void *)hdr6);
	if (error)
		return error;

	return do_out ? move_pointers_out(in, out, sizeof(struct iphdr)) : 0;
}

static void backup_pointers(struct packet *pkt, struct bkp_skb *bkp)
{
	bkp->pulled = pkt_hdrs_len(pkt);
	bkp->offset.l3 = skb_network_offset(pkt->skb);
	bkp->offset.l4 = skb_transport_offset(pkt->skb);
	bkp->payload = pkt->payload_offset;
	bkp->l4_proto = pkt_l4_proto(pkt);
}

static void restore_pointers(struct packet *pkt, struct bkp_skb *bkp)
{
	skb_push(pkt->skb, bkp->pulled);
	skb_set_network_header(pkt->skb, bkp->offset.l3);
	skb_set_transport_header(pkt->skb, bkp->offset.l4);
	pkt->payload_offset = bkp->payload;
	pkt->l4_proto = bkp->l4_proto;
	pkt->is_inner = 0;
}

verdict become_inner_packet(struct xlation *state, struct bkp_skb_tuple *bkp,
		bool do_out)
{
	struct packet *in = &state->in;
	struct packet *out = &state->out;

	backup_pointers(in, &bkp->in);
	if (do_out)
		backup_pointers(out, &bkp->out);

	switch (pkt_l3_proto(in)) {
	case L3PROTO_IPV4:
		if (move_pointers4(in, out, do_out))
			return drop(state, JSTAT_UNKNOWN);
		break;
	case L3PROTO_IPV6:
		if (move_pointers6(in, out, do_out))
			return drop(state, JSTAT_UNKNOWN);
		break;
	}

	return VERDICT_CONTINUE;
}

void restore_outer_packet(struct xlation *state, struct bkp_skb_tuple *bkp,
		bool do_out)
{
	restore_pointers(&state->in, &bkp->in);
	if (do_out)
		restore_pointers(&state->out, &bkp->out);
}

verdict xlat_l4_function(struct xlation *state,
		struct translation_steps const *steps)
{
	switch (state->in.l4_proto) {
	case L4PROTO_TCP:
		return steps->xlat_tcp(state);
	case L4PROTO_UDP:
		return steps->xlat_udp(state);
	case L4PROTO_ICMP:
		return steps->xlat_icmp(state);
	case L4PROTO_OTHER:
		return VERDICT_CONTINUE;
	}

	WARN(1, "Unknown l4 proto: %u", state->in.l4_proto);
	return drop(state, JSTAT_UNKNOWN);
}

verdict ttpcomm_translate_inner_packet(struct xlation *state,
		struct translation_steps const *steps)
{
	struct bkp_skb_tuple bkp;
	verdict result;

	result = become_inner_packet(state, &bkp, true);
	if (result != VERDICT_CONTINUE)
		return result;

	result = steps->xlat_inner_l3(state);
	if (result == VERDICT_UNTRANSLATABLE) {
		/*
		 * Accepting because of an inner packet doesn't make sense.
		 * Also we couldn't have translated this inner packet.
		 */
		result = VERDICT_DROP;
		goto end;
	}
	if (result != VERDICT_CONTINUE)
		goto end;

	result = xlat_l4_function(state, steps);
	if (result == VERDICT_UNTRANSLATABLE)
		result = VERDICT_DROP;

end:
	restore_outer_packet(state, &bkp, true);
	return result;
}

/**
 * partialize_skb - set up @out_skb so the layer 4 checksum will be computed
 * from almost-scratch by the OS or by the NIC later.
 * @csum_offset: The checksum field's offset within its header.
 *
 * When the incoming skb's ip_summed field is NONE, UNNECESSARY or COMPLETE,
 * the checksum is defined, in the sense that its correctness consistently
 * dictates whether the packet is corrupted or not. In these cases, Jool is
 * supposed to update the checksum with the translation changes (pseudoheader
 * and transport header) and forget about it. The incoming packet's corruption
 * will still be reflected in the outgoing packet's checksum.
 *
 * On the other hand, when the incoming skb's ip_summed field is PARTIAL,
 * the existing checksum only covers the pseudoheader (which Jool replaces).
 * In these cases, fully updating the checksum is wrong because it doesn't
 * already cover the transport header, and fully computing it again is wasted
 * time because this work can be deferred to the NIC (which'll likely do it
 * faster).
 *
 * The correct thing to do is convert the partial (pseudoheader-only) checksum
 * into a translated-partial (pseudoheader-only) checksum, and set up some skb
 * fields so the NIC can do its thing.
 *
 * This function handles the skb fields setting part.
 */
void partialize_skb(struct sk_buff *out_skb, __u16 csum_offset)
{
	out_skb->ip_summed = CHECKSUM_PARTIAL;
	out_skb->csum_start = skb_transport_header(out_skb) - out_skb->head;
	out_skb->csum_offset = csum_offset;
}

static verdict fix_ie(struct xlation *state, size_t in_ie_offset,
		size_t ipl, size_t pad, size_t iel)
{
	struct sk_buff *skb_old;
	struct sk_buff *skb_new;
	unsigned int ohl; /* Outer Headers Length */
	void *beginning;
	void *to;
	int offset;
	int len;
	int error;

	skb_old = state->out.skb;
	ohl = pkt_hdrs_len(&state->out);
	len = ohl + ipl + pad + iel;
	skb_new = alloc_skb(LL_MAX_HEADER + len, GFP_ATOMIC);
	if (!skb_new)
		return drop(state, JSTAT_ENOMEM);

	skb_reserve(skb_new, LL_MAX_HEADER);
	beginning = skb_put(skb_new, len);
	skb_reset_mac_header(skb_new);
	skb_reset_network_header(skb_new);
	skb_set_transport_header(skb_new, skb_transport_offset(skb_old));

	/* Outer headers */
	offset = skb_network_offset(skb_old);
	to = beginning;
	len = ohl;
	error = skb_copy_bits(skb_old, offset, to, len);
	if (error)
		goto copy_fail;

	/* Internal packet */
	offset += len;
	to += len; /* alloc_skb() always creates linear packets. */
	len = ipl;
	error = skb_copy_bits(skb_old, offset, to, len);
	if (error)
		goto copy_fail;

	if (iel) {
		/* Internal packet padding */
		to += len;
		len = pad;
		memset(to, 0, len);

		/* ICMP Extension */
		offset = in_ie_offset;
		to += len;
		len = iel;
		error = skb_copy_bits(state->in.skb, offset, to, len);
		if (error)
			goto copy_fail;
	}

	skb_dst_set(skb_new, dst_clone(skb_dst(skb_old)));
	kfree_skb(skb_old);
	state->out.skb = skb_new;
	return VERDICT_CONTINUE;

copy_fail:
	log_debug(state, "skb_copy_bits(skb, %d, %zd, %d) threw error %d.",
			offset, to - beginning, len, error);
	return drop(state, JSTAT_UNKNOWN);
}

/**
 * "Handle the ICMP Extension" in this context means
 *
 * - Make sure it aligns in accordance with the target protocol's ICMP length
 *   field. (32 bits in IPv4, 64 bits in IPv6)
 * - Make sure it fits in the packet in accordance with the target protocol's
 *   official maximum ICMP error size. (576 for IPv4, 1280 for IPv6)
 * 	- If it doesn't fit, remove it completely.
 * 	- If it does fit, trim the Optional Part if needed.
 * - Add padding to the internal packet if necessary.
 *
 * Again, see /test/graybox/test-suite/siit/7915/README.md#ic.
 *
 * "Handle the ICMP Extension" does NOT mean:
 *
 * - Translate the contents. (Jool treats extensions like opaque bit strings.)
 * - Update outer packet's L3 checksums and lengths. (Too difficult to do here;
 *   caller's responsibility.) This includes the ICMP header length.
 *
 * If this function succeeds, it will return the value of the ICMP header length
 * in args->ipl.
 */
verdict handle_icmp_extension(struct xlation *state,
		struct icmpext_args *args)
{
	struct packet *in;
	struct packet *out;
	size_t payload_len; /* Incoming packet's payload length */
	size_t in_iel; /* Incoming packet's IE length */
	size_t max_iel; /* Maximum outgoing packet's allowable IE length */
	size_t in_ieo; /* Incoming packet's IE offset */
	size_t out_ipl; /* Outgoing packet's internal packet length */
	size_t out_pad; /* Outgoing packet's padding length */
	size_t out_iel; /* Outgoing packet's IE length */

	in = &state->in;
	out = &state->out;

	/* Validate input */
	if (args->ipl == 0)
		return VERDICT_CONTINUE;
	/*
	 * There used to be a validation here, dropping packets whose args->ipl
	 * was less than 128. RFC4884 requires the essential part of ICMP
	 * extension'd packets to length >= 128, but certain Internet routers
	 * break this rule, and this in turn breaks traceroutes.
	 * https://github.com/NICMx/Jool/issues/396
	 *
	 * Current implementation: Translate < 128 incorrect unpadded packets
	 * into 128 correct padded packets.
	 */

	payload_len = in->skb->len - pkt_hdrs_len(in);
	if (args->ipl == payload_len) {
		args->ipl = 0;
		return VERDICT_CONTINUE; /* Whatever, I guess */
	}
	if (args->ipl > payload_len) {
		log_debug(state, "ICMP Length %zu > L3 payload %zu", args->ipl,
				payload_len);
		return drop(state, JSTAT_ICMPEXT_BIG);
	}

	/* Compute helpers */
	in_ieo = pkt_hdrs_len(in) + args->ipl;
	in_iel = in->skb->len - in_ieo;
	max_iel = args->max_pkt_len - (pkt_hdrs_len(out) + 128);

	/* Figure out what we want to do */
	/* (Assumption: In packet's iel equals current out packet's iel) */
	if (args->force_remove_ie || (in_iel > max_iel)) {
		out_ipl = min(out->skb->len - in_iel, args->max_pkt_len)
				- pkt_hdrs_len(out);
		out_pad = 0;
		out_iel = 0;
		args->ipl = 0;
	} else {
		out_ipl = min((size_t)out->skb->len, args->max_pkt_len) - in_iel
				- pkt_hdrs_len(out);
		/* Note to self: Yes, truncate. It's already maximized;
		 * we can't add any zeroes. Just make it fit. */
		out_ipl &= (~(size_t)0) << args->out_bits;
		out_pad = (out_ipl < 128) ? (128 - out_ipl) : 0;
		out_iel = in_iel;
		args->ipl = (out_ipl + out_pad) >> args->out_bits;
	}

	/* Move everything around */
	return fix_ie(state, skb_network_offset(in->skb) + in_ieo, out_ipl,
			out_pad, out_iel);
}

void skb_cleanup_copy(struct sk_buff *skb)
{
	/* https://github.com/NICMx/Jool/issues/289 */
#if LINUX_VERSION_AT_LEAST(5, 4, 0, 9, 0)
	nf_reset_ct(skb);
#else
	nf_reset(skb);
#endif

	/* https://github.com/NICMx/Jool/issues/400 */
#if LINUX_VERSION_AT_LEAST(5, 18, 0, 9999, 9)
	skb_clear_tstamp(skb);
#else
	skb->tstamp = 0;
#endif
}
