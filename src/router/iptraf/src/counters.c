/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

#include "counters.h"

void update_pkt_counter(struct pkt_counter *count, int bytes)
{
	if (count) {
		count->pc_packets++;
		count->pc_bytes += bytes;
	}
}

void update_proto_counter(struct proto_counter *proto_counter, int outgoing, int bytes)
{
	if (proto_counter) {
		update_pkt_counter(&proto_counter->proto_total, bytes);
		if (outgoing)
			update_pkt_counter(&proto_counter->proto_out, bytes);
		else
			update_pkt_counter(&proto_counter->proto_in, bytes);
	}
}
