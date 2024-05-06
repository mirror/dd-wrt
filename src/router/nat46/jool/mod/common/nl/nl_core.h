#ifndef SRC_MOD_COMMON_NL_CORE_H_
#define SRC_MOD_COMMON_NL_CORE_H_

#include <net/genetlink.h>

struct jool_response {
	struct genl_info *info; /* Request */
	struct sk_buff *skb; /* Packet to userspace */
	struct joolnlhdr *hdr; /* Quick access to @skb's Jool header */
	unsigned int initial_len;
};

struct xlator;

int jresponse_init(struct jool_response *response, struct genl_info *info);
int jresponse_send(struct jool_response *response);
void jresponse_cleanup(struct jool_response *response);

void jresponse_enable_m(struct jool_response *response);
int jresponse_send_array(struct xlator *jool, struct jool_response *response,
		int error);
int jresponse_send_simple(struct xlator *jool, struct genl_info *info,
		int error);


#endif /* SRC_MOD_COMMON_NL_CORE_H_ */
