#ifndef SRC_MOD_NAT64_POOL4_EMPTY_H_
#define SRC_MOD_NAT64_POOL4_EMPTY_H_

#include "common/types.h"
#include "mod/common/packet.h"

bool pool4empty_contains(struct net *ns, const struct ipv4_transport_addr *addr);
verdict pool4empty_find(struct xlation *state, struct ipv4_range *range);

#endif /* SRC_MOD_NAT64_POOL4_EMPTY_H_ */
