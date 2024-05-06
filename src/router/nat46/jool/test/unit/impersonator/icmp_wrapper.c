#include "mod/common/icmp_wrapper.h"

#include "mod/common/log.h"

/* The unit tests never spawn threads, so this does not need protection. */
static int sent = 0;

bool icmp64_send6(struct xlator *jool, struct sk_buff *skb,
		icmp_error_code error, __u32 info)
{
	return icmp64_send(jool, skb, error, info);
}

bool icmp64_send4(struct xlator *jool, struct sk_buff *skb,
		icmp_error_code error, __u32 info)
{
	return icmp64_send(jool, skb, error, info);
}

bool icmp64_send(struct xlator *jool, struct sk_buff *skb,
		icmp_error_code error, __u32 info)
{
	log_debug(jool, "Pretending I'm sending an ICMP error.");
	sent++;
	return true;
}

int icmp64_pop(void)
{
	int result = sent;
	sent = 0;
	return result;
}
