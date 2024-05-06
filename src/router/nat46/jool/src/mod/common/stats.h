#ifndef SRC_MOD_COMMON_STATS_H_
#define SRC_MOD_COMMON_STATS_H_

#include "common/stats.h"
#include "mod/common/packet.h"

struct jool_stats;

struct jool_stats *jstat_alloc(void);
void jstat_get(struct jool_stats *stats);
void jstat_put(struct jool_stats *stats);

void jstat_inc(struct jool_stats *stats, enum jool_stat_id stat);
void jstat_dec(struct jool_stats *stats, enum jool_stat_id stat);
void jstat_add(struct jool_stats *stats, enum jool_stat_id stat, int addend);

__u64 *jstat_query(struct jool_stats *stats);

#ifdef UNIT_TESTING
int jstat_refcount(struct jool_stats *stats);
#endif

#endif /* SRC_MOD_COMMON_STATS_H_ */
