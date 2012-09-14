#ifndef IPTRAF_NG_COUNTERS_H
#define IPTRAF_NG_COUNTERS_H

struct pkt_counter {
	unsigned long long pc_packets;
	unsigned long long pc_bytes;
};

struct proto_counter {
	struct pkt_counter proto_total;
	struct pkt_counter proto_in;
	struct pkt_counter proto_out;
};

void update_pkt_counter(struct pkt_counter *count, int bytes);
void update_proto_counter(struct proto_counter *proto_counter, int outgoing,
			  int bytes);

#endif	/* IPTRAF_NG_COUNTERS_H */
