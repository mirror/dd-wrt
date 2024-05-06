#include "types.h"

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

const char *l3proto_to_string(l3_protocol l3_proto)
{
	switch (l3_proto) {
	case L3PROTO_IPV6:
		return "IPv6";
	case L3PROTO_IPV4:
		return "IPv4";
	}

	return NULL;
}

const char *l4proto_to_string(l4_protocol l4_proto)
{
	switch (l4_proto) {
	case L4PROTO_TCP:
		return "TCP";
	case L4PROTO_UDP:
		return "UDP";
	case L4PROTO_ICMP:
		return "ICMP";
	case L4PROTO_OTHER:
		return "unknown";
	}

	return NULL;
}

l4_protocol str_to_l4proto(char *str)
{
	if (strcasecmp("TCP", str) == 0)
		return L4PROTO_TCP;
	if (strcasecmp("UDP", str) == 0)
		return L4PROTO_UDP;
	if (strcasecmp("ICMP", str) == 0)
		return L4PROTO_ICMP;
	return L4PROTO_OTHER;
}

bool port_range_equals(const struct port_range *r1,
		const struct port_range *r2)
{
	return (r1->min == r2->min) && (r1->max == r2->max);
}

/**
 * Range [1,3] touches [2,6].
 * Range [1,3] touches [3,6].
 * Range [1,3] touches [4,6].
 * Range [1,3] does not touch [5,6].
 */
bool port_range_touches(const struct port_range *r1,
		const struct port_range *r2)
{
	return r1->max >= (r2->min - 1) && r1->min <= (r2->max + 1);
}

bool port_range_contains(const struct port_range *range, __u16 port)
{
	return range->min <= port && port <= range->max;
}

unsigned int port_range_count(const struct port_range *range)
{
	return range->max - range->min + 1U;
}

void port_range_fuse(struct port_range *r1, const struct port_range *r2)
{
	r1->min = (r1->min < r2->min) ? r1->min : r2->min;
	r1->max = (r1->max > r2->max) ? r1->max : r2->max;
}

bool ipv4_range_equals(struct ipv4_range const *r1, struct ipv4_range const *r2)
{
	return r1->prefix.addr.s_addr == r2->prefix.addr.s_addr
			&& r1->prefix.len == r2->prefix.len
			&& port_range_equals(&r1->ports, &r2->ports);
}

bool ipv4_range_touches(struct ipv4_range const *r1, struct ipv4_range const *r2)
{
	/* TODO (fine) technically inconsistent, but fine for now */
	return r1->prefix.addr.s_addr == r2->prefix.addr.s_addr
			&& r1->prefix.len == r2->prefix.len
			&& port_range_touches(&r1->ports, &r2->ports);
}
