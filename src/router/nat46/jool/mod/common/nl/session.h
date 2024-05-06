#ifndef SRC_MOD_COMMON_NL_SESSION_H_
#define SRC_MOD_COMMON_NL_SESSION_H_

#include <net/genetlink.h>

int handle_session_foreach(struct sk_buff *skb, struct genl_info *info);

#endif /* SRC_MOD_COMMON_NL_SESSION_H_ */
