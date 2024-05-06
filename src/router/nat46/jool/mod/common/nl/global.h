#ifndef SRC_MOD_COMMON_NL_GLOBAL_H_
#define SRC_MOD_COMMON_NL_GLOBAL_H_

#include <net/genetlink.h>
#include "common/config.h"

int handle_global_foreach(struct sk_buff *skb, struct genl_info *info);
int handle_global_update(struct sk_buff *skb, struct genl_info *info);

int global_update(struct jool_globals *cfg, xlator_type xt, bool force,
		struct nlattr *root);

#endif /* SRC_MOD_COMMON_NL_GLOBAL_H_ */
