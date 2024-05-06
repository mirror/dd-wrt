#ifndef SRC_MOD_COMMON_NL_STATS_H_
#define SRC_MOD_COMMON_NL_STATS_H_

#include <net/genetlink.h>

int handle_stats_foreach(struct sk_buff *jool, struct genl_info *info);

#endif /* SRC_MOD_COMMON_NL_STATS_H_ */
