#ifndef SRC_MOD_COMMON_NL_POOL4_H_
#define SRC_MOD_COMMON_NL_POOL4_H_

#include <net/genetlink.h>

int handle_pool4_foreach(struct sk_buff *skb, struct genl_info *info);
int handle_pool4_add(struct sk_buff *skb, struct genl_info *info);
int handle_pool4_rm(struct sk_buff *skb, struct genl_info *info);
int handle_pool4_flush(struct sk_buff *skb, struct genl_info *info);

#endif /* SRC_MOD_COMMON_NL_POOL4_H_ */
