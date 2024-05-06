#ifndef SRC_MOD_COMMON_NL_HANDLER_H_
#define SRC_MOD_COMMON_NL_HANDLER_H_

/**
 * @file
 * The NAT64's layer/bridge towards the user. S/he can control its behavior
 * using this.
 */

#include <linux/skbuff.h>
#include <net/genetlink.h>

int nlhandler_setup(void);
void nlhandler_teardown(void);

int handle_jool_message(struct sk_buff *skb, struct genl_info *info);

u32 jnl_gid(void);
struct genl_family *jnl_family(void);

#endif /* SRC_MOD_COMMON_NL_HANDLER_H_ */
