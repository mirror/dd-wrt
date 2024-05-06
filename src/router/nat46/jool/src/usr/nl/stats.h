#ifndef SRC_USR_NL_STATS_H_
#define SRC_USR_NL_STATS_H_

#include "common/stats.h"
#include "usr/nl/core.h"

struct joolnl_stat_metadata {
	enum jool_stat_id id;
	char *name;
	char *doc;
};

struct joolnl_stat {
	struct joolnl_stat_metadata meta;
	__u64 value;
};

typedef struct jool_result (*joolnl_stats_foreach_cb)(
	struct joolnl_stat const *entry, void *args
);
struct jool_result joolnl_stats_foreach(
	struct joolnl_socket *sk,
	char const *iname,
	joolnl_stats_foreach_cb cb,
	void *args
);

#endif /* SRC_USR_NL_STATS_H_ */
