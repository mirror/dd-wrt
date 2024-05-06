#ifndef SRC_MOD_COMMON_NL_COMMON_H_
#define SRC_MOD_COMMON_NL_COMMON_H_

#include <net/genetlink.h>
#include "common/config.h"
#include "mod/common/xlator.h"

char *get_iname(struct genl_info *info);
struct joolnlhdr *get_jool_hdr(struct genl_info *info);

int request_handle_start(struct genl_info *info, xlator_type xt,
		struct xlator *jool, bool require_net_admin);
void request_handle_end(struct xlator *jool);

#endif /* SRC_MOD_COMMON_NL_COMMON_H_ */
