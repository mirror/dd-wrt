#ifndef SRC_MOD_COMMON_NL_ATOMIC_CONFIG_H_
#define SRC_MOD_COMMON_NL_ATOMIC_CONFIG_H_

#include <net/genetlink.h>

int handle_atomconfig_request(struct sk_buff *skb, struct genl_info *info);

#endif /* SRC_MOD_COMMON_NL_ATOMIC_CONFIG_H_ */
