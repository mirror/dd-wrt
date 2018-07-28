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

struct ip_counter {
	struct pkt_counter ip_total;
	struct pkt_counter ip_in;
	struct pkt_counter ip_out;
};
  
void pkt_counter_update(struct pkt_counter *count, int bytes);
void pkt_counter_reset(struct pkt_counter *count);

void proto_counter_update(struct proto_counter *proto_counter, int outgoing,
			  int bytes);
void proto_counter_reset(struct proto_counter *proto_counter);


void ip_counter_update(struct ip_counter *ip_counter, int outgoing, int bytes);

void ip_counter_reset(struct ip_counter *ip_counter);


#endif	/* IPTRAF_NG_COUNTERS_H */
