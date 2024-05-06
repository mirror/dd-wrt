#include "mod/common/joold.h"
#include "mod/common/db/pool4/db.h"
#include "mod/common/db/bib/db.h"
#include "mod/common/steps/compute_outgoing_tuple.h"
#include "mod/common/steps/determine_incoming_tuple.h"
#include "mod/common/steps/filtering_and_updating.h"

/**
 * @file
 * NAT64-specific functions, as linked by SIIT code.
 *
 * Most of these are supposed to be unreachable code, so they're very noisy on
 * the kernel log.
 */

static int fail(const char *function_name)
{
	WARN(true, "%s() was called from SIIT code.", function_name);
	return -EINVAL;
}

verdict determine_in_tuple(struct xlation *state)
{
	fail(__func__);
	return VERDICT_DROP;
}

verdict filtering_and_updating(struct xlation *state)
{
	fail(__func__);
	return VERDICT_DROP;
}

verdict compute_out_tuple(struct xlation *state)
{
	fail(__func__);
	return VERDICT_DROP;
}

struct joold_queue *joold_alloc(void)
{
	fail(__func__);
	return NULL;
}

void joold_get(struct joold_queue *queue)
{
	fail(__func__);
}

void joold_put(struct joold_queue *queue)
{
	fail(__func__);
}

struct pool4 *pool4db_alloc(void)
{
	fail(__func__);
	return NULL;
}

void pool4db_get(struct pool4 *pool)
{
	fail(__func__);
}

void pool4db_put(struct pool4 *pool)
{
	fail(__func__);
}

struct bib *bib_alloc(void)
{
	fail(__func__);
	return NULL;
}

void bib_get(struct bib *db)
{
	fail(__func__);
}

void bib_put(struct bib *db)
{
	fail(__func__);
}

bool is_hairpin_nat64(struct xlation *state)
{
	fail(__func__);
	return false;
}

verdict handling_hairpinning_nat64(struct xlation *old)
{
	fail(__func__);
	return VERDICT_DROP;
}
