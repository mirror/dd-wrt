#ifndef SRC_MOD_COMMON_CONFIG_H_
#define SRC_MOD_COMMON_CONFIG_H_

#include "common/config.h"
#include "common/global.h"

int globals_init(struct jool_globals *config, xlator_type type,
		struct ipv6_prefix *pool6);

int globals_foreach(struct jool_globals *config,
		xlator_type xt,
		int (*cb)(struct joolnl_global_meta const *, void *, void *),
		void *arg,
		enum joolnl_attr_global offset);

int pool6_validate(struct config_prefix6 *prefix, bool force);

#endif /* SRC_MOD_COMMON_CONFIG_H_ */
