#include "framework/send_packet.h"

#include "mod/common/log.h"

struct sk_buff *skb_out = NULL;

verdict sendpkt_send(struct xlation *state)
{
	log_debug(state, "Pretending I'm sending a packet.");
	skb_out = state->out.skb;
	return VERDICT_CONTINUE;
}
