#ifndef SRC_MOD_COMMON_NL_JOOLD_H_
#define SRC_MOD_COMMON_NL_JOOLD_H_

#include <net/genetlink.h>
#include "mod/common/xlator.h"

int handle_joold_add(struct sk_buff *skb, struct genl_info *info);
int handle_joold_advertise(struct sk_buff *skb, struct genl_info *info);
int handle_joold_ack(struct sk_buff *skb, struct genl_info *info);

#endif /* SRC_MOD_COMMON_NL_JOOLD_H_ */
