#ifndef SRC_MOD_COMMON_ATOMIC_CONFIG_H_
#define SRC_MOD_COMMON_ATOMIC_CONFIG_H_

#include <net/genetlink.h>

void atomconfig_teardown(void);
int atomconfig_add(struct sk_buff *skb, struct genl_info *info);

#endif /* SRC_MOD_COMMON_ATOMIC_CONFIG_H_ */
