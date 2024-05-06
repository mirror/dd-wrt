#ifndef SRC_MOD_COMMON_NL_ADDRESS_H_
#define SRC_MOD_COMMON_NL_ADDRESS_H_

#include <net/genetlink.h>
#include "common/config.h"

int handle_address_query64(struct sk_buff *skb, struct genl_info *info);
int handle_address_query46(struct sk_buff *skb, struct genl_info *info);

#endif /* SRC_MOD_COMMON_NL_ADDRESS_H_ */
