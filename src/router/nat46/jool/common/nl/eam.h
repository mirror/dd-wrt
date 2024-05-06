#ifndef SRC_MOD_COMMON_NL_EAM_H_
#define SRC_MOD_COMMON_NL_EAM_H_

#include <net/genetlink.h>

int handle_eamt_foreach(struct sk_buff *skb, struct genl_info *info);
int handle_eamt_add(struct sk_buff *skb, struct genl_info *info);
int handle_eamt_rm(struct sk_buff *skb, struct genl_info *info);
int handle_eamt_flush(struct sk_buff *skb, struct genl_info *info);

#endif /* SRC_MOD_COMMON_NL_EAM_H_ */
