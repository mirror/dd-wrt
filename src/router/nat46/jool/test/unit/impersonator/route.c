#include "mod/common/route.h"

#include "mod/common/dev.h"
#include "mod/common/log.h"
#include "framework/unit_test.h"

struct dst_entry *route4(struct xlator *jool, struct flowi4 *flow)
{
	log_debug(jool, "Pretending I'm routing an IPv4 packet.");
	return NULL;
}

struct dst_entry *route6(struct xlator *jool, struct flowi6 *flow)
{
	log_debug(jool, "Pretending I'm routing an IPv6 packet.");
	return NULL;
}
