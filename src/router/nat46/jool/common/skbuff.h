#ifndef SRC_MOD_COMMON_SKBUFF_H_
#define SRC_MOD_COMMON_SKBUFF_H_

#include <linux/skbuff.h>

void skb_log(struct sk_buff *skb, char *label);

#endif /* SRC_MOD_COMMON_SKBUFF_H_ */
