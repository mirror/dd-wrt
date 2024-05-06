#include "mod/common/steps/handling_hairpinning_nat64.h"

#include "mod/common/log.h"
#include "mod/common/rfc7915/core.h"
#include "mod/common/steps/send_packet.h"
#include "mod/common/steps/compute_outgoing_tuple.h"
#include "mod/common/steps/filtering_and_updating.h"
#include "mod/common/db/pool4/db.h"


/**
 * Checks whether "pkt" is a hairpin packet.
 *
 * @param pkt outgoing packet the NAT64 would send if it's not a hairpin.
 * @return whether pkt is a hairpin packet.
 */
bool is_hairpin_nat64(struct xlation *state)
{
	if (state->out.tuple.l3_proto == L3PROTO_IPV6)
		return false;

	/*
	 * This collides with RFC 6146.
	 * The RFC says "packet (...) destination address", but I'm using
	 * "tuple destination address".
	 * I mean you can throw tomatoes, but this makes lot more sense to me.
	 * Otherwise Jool would hairpin ICMP errors that were actually intended
	 * for its node. It might take a miracle for these packets to exist,
	 * but hey, why the hell not.
	 */
	return pool4db_contains(state->jool.nat64.pool4, state->jool.ns,
			state->out.tuple.l4_proto, &state->out.tuple.dst.addr4);
}

/**
 * Mirrors the core's behavior by processing skb_in as if it was the incoming packet.
 *
 * @param skb_in the outgoing packet. Except because it's a hairpin, here it's treated as if it was
 *		the one received from the network.
 * @param tuple_in skb_in's tuple.
 * @return whether we managed to U-turn the packet successfully.
 */
verdict handling_hairpinning_nat64(struct xlation *old)
{
	struct xlation *new;
	verdict result;

	log_debug(old, "Step 5: Handling Hairpinning...");

	new = xlation_create(&old->jool);
	if (!new)
		return VERDICT_DROP;
	new->in = old->out;
	new->is_hairpin = true;

	result = filtering_and_updating(new);
	if (result != VERDICT_CONTINUE)
		goto end;
	result = compute_out_tuple(new);
	if (result != VERDICT_CONTINUE)
		goto end;
	result = translating_the_packet(new);
	if (result != VERDICT_CONTINUE)
		goto end;
	result = sendpkt_send(new);
	if (result != VERDICT_CONTINUE)
		goto end;

	log_debug(old, "Done step 5.");
end:	xlation_destroy(new);
	return result;
}
