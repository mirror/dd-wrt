#include "mod/common/trace.h"

#include "mod/common/log.h"

void pkt_trace4(struct xlation *state)
{
	union {
		struct tcphdr *tcp;
		struct udphdr *udp;
		struct icmphdr *icmp;
	} ptr;

	switch (pkt_l4_proto(&state->in)) {
	case L4PROTO_TCP:
		if (is_first_frag4(pkt_ip4_hdr(&state->in))) {
			ptr.tcp = pkt_tcp_hdr(&state->in);
			log_debug(state, "TCP %u->%u",
					be16_to_cpu(ptr.tcp->source),
					be16_to_cpu(ptr.tcp->dest));
		} else {
			log_debug(state, "TCP (Fragment; ports unavailable.)");
		}
		break;
	case L4PROTO_UDP:
		if (is_first_frag4(pkt_ip4_hdr(&state->in))) {
			ptr.udp = pkt_udp_hdr(&state->in);
			log_debug(state, "UDP %u->%u",
					be16_to_cpu(ptr.udp->source),
					be16_to_cpu(ptr.udp->dest));
		} else {
			log_debug(state, "UDP (Fragment; ports unavailable.)");
		}
		break;
	case L4PROTO_ICMP:
		ptr.icmp = pkt_icmp4_hdr(&state->in);
		log_debug(state, "ICMPv4 type:%u code:%u id:%u",
				ptr.icmp->type, ptr.icmp->code,
				be16_to_cpu(ptr.icmp->un.echo.id));
		break;
	case L4PROTO_OTHER:
		log_debug(state, "Unknown l4 protocol");
	}
}

void pkt_trace6(struct xlation *state)
{
	union {
		struct tcphdr *tcp;
		struct udphdr *udp;
		struct icmp6hdr *icmp;
	} ptr;

	switch (pkt_l4_proto(&state->in)) {
	case L4PROTO_TCP:
		if (is_first_frag6(pkt_frag_hdr(&state->in))) {
			ptr.tcp = pkt_tcp_hdr(&state->in);
			log_debug(state, "TCP %u->%u",
					be16_to_cpu(ptr.tcp->source),
					be16_to_cpu(ptr.tcp->dest));
		} else {
			log_debug(state, "TCP (Fragment; ports unavailable.)");
		}
		break;
	case L4PROTO_UDP:
		if (is_first_frag6(pkt_frag_hdr(&state->in))) {
			ptr.udp = pkt_udp_hdr(&state->in);
			log_debug(state, "UDP %u->%u",
					be16_to_cpu(ptr.udp->source),
					be16_to_cpu(ptr.udp->dest));
		} else {
			log_debug(state, "UDP (Fragment; ports unavailable.)");
		}
		break;
	case L4PROTO_ICMP:
		ptr.icmp = pkt_icmp6_hdr(&state->in);
		log_debug(state, "ICMPv6 type:%u code:%u id:%u",
				ptr.icmp->icmp6_type, ptr.icmp->icmp6_code,
				be16_to_cpu(ptr.icmp->icmp6_identifier));
		break;
	case L4PROTO_OTHER:
		log_debug(state, "Unknown l4 protocol");
	}
}
