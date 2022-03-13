/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

#include "iptraf-ng-compat.h"

#include "counters.h"

void pkt_counter_update(struct pkt_counter *count, int bytes)
{
	if (count) {
		count->pc_packets++;
		count->pc_bytes += bytes;
	}
}

void pkt_counter_reset(struct pkt_counter *count)
{
	if (count)
		memset(count, 0, sizeof(*count));
}

void proto_counter_update(struct proto_counter *proto_counter, int outgoing, int bytes)
{
	if (proto_counter) {
		pkt_counter_update(&proto_counter->proto_total, bytes);
		if (outgoing)
			pkt_counter_update(&proto_counter->proto_out, bytes);
		else
			pkt_counter_update(&proto_counter->proto_in, bytes);
	}
}

void proto_counter_reset(struct proto_counter *proto_counter)
{
	if (proto_counter) {
		pkt_counter_reset(&proto_counter->proto_total);
		pkt_counter_reset(&proto_counter->proto_out);
		pkt_counter_reset(&proto_counter->proto_in);
	}
}
