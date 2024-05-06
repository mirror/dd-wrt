#ifndef SRC_COMMON_IPTABLES_H_
#define SRC_COMMON_IPTABLES_H_

#include "common/types.h"

#define IPTABLES_SIIT_MODULE_NAME "JOOL_SIIT"
#define IPTABLES_NAT64_MODULE_NAME "JOOL"

/* Mind alignment on this structure. */
struct target_info {
	char iname[INAME_MAX_SIZE];
	__u8 type; /* xlator_type */
};

#endif /* SRC_COMMON_IPTABLES_H_ */
