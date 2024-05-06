#include "mod/common/stats.h"

static struct jool_stats {
	int junk;
} phony;

struct jool_stats *jstat_alloc(void)
{
	return &phony;
}

void jstat_get(struct jool_stats *stats)
{
	/* No code. */
}

void jstat_put(struct jool_stats *stats)
{
	/* No code. */
}

void jstat_inc(struct jool_stats *stats, enum jool_stat_id stat)
{
	/* No code. */
}

void jstat_dec(struct jool_stats *stats, enum jool_stat_id stat)
{
	/* No code. */
}

void jstat_add(struct jool_stats *stats, enum jool_stat_id stat, int addend)
{
	/* No code. */
}
