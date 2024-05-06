#ifndef SRC_USR_ARGP_DNS_H_
#define SRC_USR_ARGP_DNS_H_

#include "common/types.h"

void print_addr6(struct ipv6_transport_addr const *addr, bool numeric,
		char const *separator, __u8 l4_proto);
void print_addr4(struct ipv4_transport_addr const *addr, bool numeric,
		char const *separator, __u8 l4_proto);

#endif /* SRC_USR_ARGP_DNS_H_ */
