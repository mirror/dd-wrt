#include "framework/types.h"

int init_tuple4(struct tuple *tuple4, char *src_addr, u16 src_port,
		char *dst_addr, u16 dst_port, l4_protocol l4_proto)
{
	int error;

	error = str_to_addr4(src_addr, &tuple4->src.addr4.l3);
	if (error)
		return error;
	tuple4->src.addr4.l4 = src_port;

	error = str_to_addr4(dst_addr, &tuple4->dst.addr4.l3);
	if (error)
		return error;
	tuple4->dst.addr4.l4 = (l4_proto != L4PROTO_ICMP) ? dst_port : src_port;

	tuple4->l3_proto = L3PROTO_IPV4;
	tuple4->l4_proto = l4_proto;

	return 0;
}

int init_tuple6(struct tuple *tuple6, char *src_addr, u16 src_port,
		char *dst_addr, u16 dst_port, l4_protocol l4_proto)
{
	int error;

	error = str_to_addr6(src_addr, &tuple6->src.addr6.l3);
	if (error)
		return error;
	tuple6->src.addr6.l4 = src_port;

	error = str_to_addr6(dst_addr, &tuple6->dst.addr6.l3);
	if (error)
		return error;
	tuple6->dst.addr6.l4 = (l4_proto != L4PROTO_ICMP) ? dst_port : src_port;

	tuple6->l3_proto = L3PROTO_IPV6;
	tuple6->l4_proto = l4_proto;

	return 0;
}
