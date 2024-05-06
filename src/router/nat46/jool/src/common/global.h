#ifndef SRC_COMMON_GLOBAL_H_
#define SRC_COMMON_GLOBAL_H_

#include "common/config.h"

#ifdef __KERNEL__
#include <linux/skbuff.h>
#else
#include "usr/util/cJSON.h"
#include "usr/util/result.h"
#endif

struct joolnl_global_meta;

struct joolnl_global_meta const *joolnl_global_id2meta(
	enum joolnl_attr_global id
);

enum joolnl_attr_global joolnl_global_meta_id(
	struct joolnl_global_meta const *meta
);
char const *joolnl_global_meta_name(struct joolnl_global_meta const *meta);
xlator_type joolnl_global_meta_xt(struct joolnl_global_meta const *meta);
char const *joolnl_global_meta_values(struct joolnl_global_meta const *meta);

#define joolnl_global_foreach_meta(pos) 				\
	for (								\
		pos = joolnl_global_meta_first();			\
		pos <= joolnl_global_meta_last();			\
		pos = joolnl_global_meta_next(pos)			\
	)

struct joolnl_global_meta const *joolnl_global_meta_first(void);
struct joolnl_global_meta const *joolnl_global_meta_last(void);
struct joolnl_global_meta const *joolnl_global_meta_next(
	struct joolnl_global_meta const *pos
);
unsigned int joolnl_global_meta_count(void);

void *joolnl_global_get(
	struct joolnl_global_meta const *meta,
	struct jool_globals *cfg
);

#ifdef __KERNEL__

int joolnl_global_raw2nl(
	struct joolnl_global_meta const *meta,
	void *raw,
	struct sk_buff *skb
);

int joolnl_global_nl2raw(
	struct joolnl_global_meta const *meta,
	struct nlattr *nl,
	void *raw,
	bool force
);

#else

struct jool_result joolnl_global_nl2raw(
	struct joolnl_global_meta const *meta,
	struct nlattr *nl,
	void *raw
);

struct jool_result joolnl_global_str2nl(
	struct joolnl_global_meta const *meta,
	char const *str,
	struct nl_msg *nl
);

struct jool_result joolnl_global_json2nl(
	struct joolnl_global_meta const *meta,
	cJSON *json,
	struct nl_msg *msg
);

void joolnl_global_print(
	struct joolnl_global_meta const *meta,
	void *value,
	bool csv
);

#endif

#endif /* SRC_COMMON_GLOBAL_H_ */
