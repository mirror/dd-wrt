#ifndef SRC_MOD_COMMON_NL_INSTANCE_H_
#define SRC_MOD_COMMON_NL_INSTANCE_H_

#include <net/genetlink.h>
#include "common/config.h"

int handle_instance_foreach(struct sk_buff *skb, struct genl_info *info);
int handle_instance_add(struct sk_buff *skb, struct genl_info *info);
int handle_instance_hello(struct sk_buff *skb, struct genl_info *info);
int handle_instance_rm(struct sk_buff *skb, struct genl_info *info);
int handle_instance_flush(struct sk_buff *skb, struct genl_info *info);

#endif /* SRC_MOD_COMMON_NL_INSTANCE_H_ */
